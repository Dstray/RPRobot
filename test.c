#include "video/capture.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define IMG_PACKAGE_SIZE 0x1000

int main(int argc, char *argv[])
{
    char* dev_name = "/dev/video0";
    enum io_method io = IO_METHOD_MMAP;
    int fd = open_device(dev_name);
    init_device(fd, dev_name, io, 4);
    start_capturing(fd, io);

    if (argc < 2)
        exception_exit("No port", "provided");
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errno_exit("opening socket failed");

    struct sockaddr_in serv_addr;
    CLEAR(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
        sizeof serv_addr) == -1)
        errno_exit("binding failed.");

    if (listen(sockfd, 5) == -1)
        errno_exit("listening failed.");

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof cli_addr;
    int newsockfd;
    if ((newsockfd = accept(sockfd, 
        (struct sockaddr *) &cli_addr, &clilen)) == -1) 
        errno_exit("accepting failed.");

    int pkg_start, pkg_left;
    struct buffer* imgbuf;
    while (1) {
        imgbuf = capture(fd, io);
        pkg_start = imgbuf->start;
        pkg_left = imgbuf->length;
        while (pkg_left > IMG_PACKAGE_SIZE) {
            send(newsockfd, pkg_start, IMG_PACKAGE_SIZE, 0);
            pkg_start += IMG_PACKAGE_SIZE;
            pkg_left -= IMG_PACKAGE_SIZE;
        }
        send(newsockfd, pkg_start, pkg_left, 0);
    }

    stop_capturing(fd, io);
    close_device(fd, io);

    return 0;
}