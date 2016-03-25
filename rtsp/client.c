#include "rtsp.h"
#include <netdb.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errno_exit("opening socket failed");

    struct hostent* server = gethostbyname(argv[1]);
    if (server == NULL)
        errno_exit("no such host");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr,
        server->h_addr,
        server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));
    printf("h_addr: %d.%d.%d.%d\n", server->h_addr[0],
        server->h_addr[1], server->h_addr[2], server->h_addr[3]);

    if (connect(sockfd, (struct sockaddr *) &serv_addr,
        sizeof serv_addr) == -1) 
        errno_exit("connecting failed");

    char buffer[RTP_PACKET_SIZE_MAX];
    memset(buffer, 0, RTP_PACKET_SIZE_MAX);

    int n;
    FILE* fd = fopen("frame.rcv", "wb");
    while (1) {
        if ((n = recv(sockfd, buffer, RTP_PACKET_SIZE_MAX, 0)) == -1) 
            errno_exit("reading from socket failed");
        if (n <= 2)
            break;
        fwrite(buffer, 1, n, fd);
    }
    fclose(fd);

    close(sockfd);
    return 0;
}