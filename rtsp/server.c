/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "../common/exception.h"
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 256

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
    memset(buffer, 0, BUFFER_SIZE);
    
    int n;
    if ((n = recv(newsockfd, buffer, BUFFER_SIZE - 1, 0)) < 0)
        errno_exit("reading from socket failed");
    printf("Here is the message: %s\n",buffer);

    if ((n = send(newsockfd,"I got your message",18, 0)) < 0)
        errno_exit("writing to socket failed");

    close(newsockfd);
    close(sockfd);
    return 0; 
}