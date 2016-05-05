#include "video/capture.h"
#include "audio/recording.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IMG_PACKAGE_SIZE 0x1000
#define WAV_PACKAGE_SIZE 0x0400
#define CTL_PACKAGE_SIZE 0x0100
#define PORT_VIDEO 555
#define PORT_CTRL (PORT_VIDEO + 10000)
#define PORT_AUDIO (PORT_CTRL + 1)

void create_response(unsigned char* pkg, int* plen, void* sig) {
    printf("len: %d\n", *plen);
    *plen = 0;
    if (pkg[1] == 1) {
        short tmp = htons(PORT_VIDEO);
        memcpy(pkg, &tmp, sizeof tmp);
        tmp = htons(PORT_AUDIO);
        memcpy(pkg + 2, &tmp, sizeof tmp);
        *plen = 2 * (2 * sizeof tmp);
    } else if (pkg[1] == 0) {
        *(char*)sig = 2;
    }
}

int main(int argc, char *argv[])
{
    // Shared Memory
    void* shm;
    int shmid;
    if ((shmid = shmget((key_t)1338, 1, 0666 | IPC_CREAT)) == -1) // IPC_EXCL
        errno_exit("shmget failed");
    if ((shm = shmat(shmid, NULL, 0)) == (void*)-1)
        errno_exit("shmat failed");
    *(char*)shm = 0;

    // Socket
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof cli_addr;
    CLEAR(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int sockfd, fd;
    char* dev_name;

    int ret, vport;/*
    if (argc < 2)
        exception_exit("No port", "provided");
    else
        vport = atoi(argv[1]);*/


    if ((ret = fork()) == -1)
        errno_exit("fork");
    else if (ret == 0) {
        if ((shm = shmat(shmid, NULL, 0)) == (void*)-1)
            errno_exit("shmat failed");
        // === Video Process ===
        dev_name = "/dev/video0";
        enum io_method io = IO_METHOD_MMAP;
        fd = open_device(dev_name);
        init_device(fd, dev_name, io, 4);
        start_capturing(fd, io);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            errno_exit("opening socket failed");

        serv_addr.sin_port = htons(PORT_VIDEO);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof serv_addr) == -1)
            errno_exit("binding failed");

        if (listen(sockfd, 5) == -1)
            errno_exit("listening failed");

        int newsockfd;
        while (*(char*)shm < 3) {
            if ((newsockfd = accept(sockfd, 
                (struct sockaddr *) &cli_addr, &cli_len)) == -1) 
                errno_exit("accepting failed");

            unsigned char* pkg_start;
            int pkg_left, seq;
            struct buffer* imgbuf;
            unsigned char pkgbuf[IMG_PACKAGE_SIZE] = {0};
            while (*(char*)shm == 0) {
                imgbuf = capture(fd, io);
                pkg_start = imgbuf->start;
                pkg_left = imgbuf->length;
                //printf("byteused: %d\n", pkg_left);
                seq = 0;
                while (pkg_left > IMG_PACKAGE_SIZE - 4) {
                    pkgbuf[0] = '$';
                    pkgbuf[1] = seq++;
                    pkgbuf[2] = ((IMG_PACKAGE_SIZE - 4) >> 8) & 0xFF;
                    pkgbuf[3] = (IMG_PACKAGE_SIZE - 4) & 0xFF;
                    //printf("seq: %d, 0x%02x%02x\n", pkgbuf[1], pkgbuf[2], pkgbuf[3]);
                    memcpy(pkgbuf + 4, pkg_start, IMG_PACKAGE_SIZE - 4);
                    send(newsockfd, pkgbuf, IMG_PACKAGE_SIZE, 0);
                    pkg_start += (IMG_PACKAGE_SIZE - 4);
                    pkg_left -= (IMG_PACKAGE_SIZE - 4);
                }
                pkgbuf[0] = '$';
                pkgbuf[1] = '^';
                pkgbuf[2] = (pkg_left >> 8) & 0xFF;
                pkgbuf[3] = pkg_left & 0xFF;
                //printf("seq: %d, 0x%02x%02x\n", pkgbuf[1], pkgbuf[2], pkgbuf[3]);
                memcpy(pkgbuf + 4, pkg_start, pkg_left);
                send(newsockfd, pkgbuf, pkg_left + 4, 0);
            }
            --shm;
        }
        close(sockfd);

        stop_capturing(fd, io);
        close_device(fd, io);
        if (shmdt(shm) == -1)
            errno_exit("shmdt failed");
        return 0; // Video Process end
    } else {
        if ((ret = fork()) == -1)
            errno_exit("fork");
        else if (ret == 0) {
            if ((shm = shmat(shmid, NULL, 0)) == (void*)-1)
                errno_exit("shmat failed");
            // === Audio Process ===
            dev_name = "/dev/dsp";
            fd = oss_open_device(dev_name);
            oss_init_device(fd, WAV_PACKAGE_SIZE);

            if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                errno_exit("opening socket failed");

            serv_addr.sin_port = htons(PORT_AUDIO);
            if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) == -1)
                errno_exit("binding failed");

            unsigned char pkgbuf[WAV_PACKAGE_SIZE] = {0};
            while (*(char*)shm < 3) {
                if (recvfrom(sockfd, pkgbuf, WAV_PACKAGE_SIZE, 0,
                    (struct sockaddr *) &cli_addr, &cli_len) == -1)
                    errno_exit("recvfrom failed");
                printf("Received packet from %s:%d\nData: %s\n", 
                    inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), pkgbuf);

                struct buffer* wavbuf;
                while (*(char*)shm == 0) {
                    wavbuf = oss_record(fd);

                    if (sendto(sockfd, wavbuf->start, wavbuf->length, 0,
                        (struct sockaddr *) &cli_addr, cli_len) == -1)
                        errno_exit("sendto failed");
                    /*
                    int level = 0, len = wavbuf->length / 2, i;
                    short *buf = (short *)wavbuf->start;
                    for (i = 0; i < len; i++) {
                        int v = buf[i];

                        if (v < 0)
                            v = -v;

                        if (v > level)
                            level = v;
                    }
                    level = (level + 1) / 1024;

                    for (i = 0; i < level; i++)
                        printf ("*");
                    for (i = level; i < 32; i++)
                        printf (".");
                    printf ("\r");
                    fflush (stdout);
                    */
                    wavbuf->length = WAV_PACKAGE_SIZE;
                }
                --shm;
            }

            close(sockfd);
            if (shmdt(shm) == -1)
                errno_exit("shmdt failed");
            return 0; // Audio Process end
        } else {
            // === Main Process ===
            if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                errno_exit("opening socket failed");

            serv_addr.sin_port = htons(PORT_CTRL);
            if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) == -1)
                errno_exit("binding failed");

            unsigned char pkgbuf[CTL_PACKAGE_SIZE] = {0};
            while (1) {
                int len;
                if ((len = recvfrom(sockfd, pkgbuf, CTL_PACKAGE_SIZE, 0,
                    (struct sockaddr *) &cli_addr, &cli_len)) == -1)
                    errno_exit("recvfrom failed");
                create_response(pkgbuf, &len, shm);
                if (len && sendto(sockfd, pkgbuf, len, 0,
                    (struct sockaddr *) &cli_addr, cli_len) == -1)
                    errno_exit("sendto failed");
            }

            if (shmdt(shm) == -1)
                errno_exit("shmdt failed");
        }
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1)
        errno_exit("shmctl failed");

    return 0;
}