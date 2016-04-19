#include "video/capture.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IMG_PACKAGE_SIZE 0x1000
#define WAV_PACKAGE_SIZE 0x0400

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof cli_addr;
    CLEAR(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int sockfd;

    int ret, vport;
    if (argc < 3)
        exception_exit("No port", "provided");
    else
        vport = atoi(argv[1]);


    if ((ret = fork()) == -1)
        errno_exit("fork");
    else if (ret == 0) {
        char* dev_name = "/dev/video0";
        enum io_method io = IO_METHOD_MMAP;
        int fd = open_device(dev_name);
        init_device(fd, dev_name, io, 4);
        start_capturing(fd, io);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            errno_exit("opening socket failed");

        serv_addr.sin_port = htons(vport);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof serv_addr) == -1)
            errno_exit("binding failed");

        if (listen(sockfd, 5) == -1)
            errno_exit("listening failed");

        int newsockfd;
        if ((newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &cli_len)) == -1) 
            errno_exit("accepting failed");

        unsigned char* pkg_start;
        int pkg_left, cnt = atoi(argv[2]), seq;
        struct buffer* imgbuf;
        unsigned char pkgbuf[IMG_PACKAGE_SIZE] = {0};
        while (cnt--) {
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

        stop_capturing(fd, io);
        close_device(fd, io);
    } else {
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
            errno_exit("opening socket failed");

        serv_addr.sin_port = htons(vport + 10000);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) == -1)
            errno_exit("binding failed");

        unsigned char pkgbuf[WAV_PACKAGE_SIZE] = {0};
        if (recvfrom(sockfd, pkgbuf, WAV_PACKAGE_SIZE, 0,
            (struct sockaddr *) &cli_addr, &cli_len) == -1)
            errno_exit("recvfrom failed");
        printf("Received packet from %s:%d\nData: %s\n", 
            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), pkgbuf);

        close(sockfd);
        wait(&ret);
    }

    return 0;
}