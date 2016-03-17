#include "rtsp.h"

#define BUFFER_SIZE 0x0400
#define find_linefeed(s, l) find_char('\n', s, l)

int find_char(char c, char* s, int l) {
    int i;
    for (i = 0; i < l; i++)
        if (s[i] == c)
            return i;
    return -1;
}

int parse_request_line(char* buf, int len,
    struct request_line* req_line) {
    if (sscanf(buf, "%s %s %s\r\n", req_line->method,
        req_line->req_uri, req_line->version) == 3)
        return find_linefeed(buf, len) + 1;
    else
        return -1;
}

int parse_headers(char* buf, int len,
    struct message_headers* m_headers) {
    char* rem = buf;
    m_headers->num = 0;
    int l_end, i_colon;
    while (1) {
        l_end = find_linefeed(rem, len);
        assert(rem[l_end - 1] == '\r');
        if (l_end == 1)
            break;

        i_colon = find_char(':', rem, l_end);
        m_headers->keys[m_headers->num] = rem;
        rem[i_colon] = '\0';
        assert(rem[i_colon + 1] == ' ');
        m_headers->values[m_headers->num] = rem + i_colon + 2;
        rem[l_end - 1] = '\0';

        rem += l_end + 1;
        m_headers->num++;
    }
    return rem - buf + 2;
}

int parse_request(char* buf, int len,
    struct request_line* req_line,
    struct message_headers* m_headers) {
    int idx = 0;
    idx += parse_request_line(buf, len, req_line);
    idx += parse_headers(&(buf[idx]), len - idx, m_headers);
    //idx = parse_message_body(buf[idx], len - idx);
    if (buf[idx] == '\r' && buf[idx + 2] == 0)
        printf("====== request ======\n");
}

void fprint_request(FILE* stream,
    struct request_line* req_line,
    struct message_headers* m_headers) {
    fprintf(stream, "%s %s %s\n", req_line->method, req_line->req_uri, req_line->version);
    int i;
    for (i = 0; i != m_headers->num; i++)
        fprintf(stream, "%s: %s\n", m_headers->keys[i], m_headers->values[i]);
}

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

    struct message_headers m_headers;
    struct request_line req_line;
    char buffer[BUFFER_SIZE];
    int n, cnt = 3;
    while (cnt --) {
        memset(buffer, 0, BUFFER_SIZE);
        if ((n = recv(newsockfd, buffer, BUFFER_SIZE - 1, 0)) == -1)
            errno_exit("reading from socket failed");
        parse_request(buffer, BUFFER_SIZE, &req_line, &m_headers);
        fprint_request(stdout, &req_line, &m_headers);
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