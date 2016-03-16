#include "rtsp.h"

#define BUFFER_SIZE 0x1000

int main(int argc, char *argv[])
{
    if (argc < 2)
        exception_exit("No port", "provided");
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errno_exit("opening socket failed");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);
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
        (struct sockaddr *) &cli_addr, 
        &clilen)) == -1) 
        errno_exit("accepting failed.");

    char buffer[BUFFER_SIZE];
    int n, cnt = 3;
    while (cnt --) {
        memset(buffer, 0, BUFFER_SIZE);
        if ((n = recv(newsockfd, buffer, BUFFER_SIZE - 1, 0)) == -1)
            errno_exit("reading from socket failed");
        printf("====== request ======\n%s\n", buffer);
        n = sprintf(buffer, "RTSP/1.0 %d %s\r\n%s%s\r\n", 200, "OK",
            "CSeq: 1\r\n", "Sever: RPRobot/1.0\r\n");
        if ((n = send(newsockfd, buffer, n, 0)) == -1)
            errno_exit("writing to socket failed");
        printf("====== response ======\n%s\n", buffer);
    }

    close(newsockfd);
    close(sockfd);
    return 0; 
}