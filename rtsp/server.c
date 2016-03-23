#include "rtsp.h"
#include <arpa/inet.h>

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
    struct request_line* p_reqline) {
    if (sscanf(buf, "%s %s %s\r\n", p_reqline->method,
        p_reqline->req_uri, p_reqline->version) == 3)
        return find_linefeed(buf, len) + 1;
    else
        return -1;
}

int parse_headers(char* buf, int len,
    struct header_buffer* p_hbuf) {
    char* rem = buf;
    int l_end, i_colon;
    while (1) {
        l_end = find_linefeed(rem, len);
        assert(rem[l_end - 1] == '\r');
        if (l_end == 1)
            break;

        i_colon = find_char(':', rem, l_end);
        rem[i_colon] = '\0';
        p_hbuf->fields[p_hbuf->num] = get_header(rem, HEADER_TYPE_REQ);
        assert(rem[i_colon + 1] == ' ');
        rem[l_end - 1] = '\0';
        p_hbuf->values[p_hbuf->num] = rem + i_colon + 2;

        rem += l_end + 1;
        p_hbuf->num++;
    }
    return rem - buf + 2;
}

int parse_request(char* buf, int len, struct request* p_req) {
    CLEAR(p_req->req_line);
    p_req->h_buf.num = 0;

    int idx = 0;
    idx += parse_request_line(buf, len, &(p_req->req_line));
    if (idx == -1)
        return -1;
    idx += parse_headers(buf + idx, len - idx, &(p_req->h_buf));
    //idx = parse_message_body(buf[idx], len - idx);
    //TODO
    return idx;
}

void fprint_request(FILE* stream, struct request* p_req) {
    fprintf(stream, "====== request ======\n");
    fprintf(stream, "%s %s %s\n", p_req->req_line.method,
        p_req->req_line.req_uri, p_req->req_line.version);
    int i;
    for (i = 0; i != p_req->h_buf.num; i++)
        fprintf(stream, "%s: %s\n",
            p_req->h_buf.fields[i]->name, p_req->h_buf.values[i]);
}

int process_request(struct request* p_req, struct response* p_res) {
    CLEAR_BUF(p_res->sta_line.version);
    int n_methods = SIZEOF(methods), i, ret;
    for (i = 0; i != n_methods; i++)
        if (!strcmp(methods[i].name, p_req->req_line.method))
            break;
    if (i < n_methods)
        ret = methods[i].func(p_req, p_res);
    else
        ret = process_method_unsupported(p_req, p_res);
    strcpy(p_res->sta_line.version, RTSP_VERSION);
    return ret;
}

int create_response_message(char* resbuf,
    struct response* p_res) {
    CLEAR_BUF(resbuf);
    int i, n = sprintf(
        resbuf, "%s %d %s\r\n",
        p_res->sta_line.version,
        p_res->sta_line.p_status->code,
        p_res->sta_line.p_status->reason_phrase);
    for (i = 0; i != p_res->h_buf.num; i++)
        n += sprintf(resbuf + n, "%s: %s\r\n",
            p_res->h_buf.fields[i]->name, p_res->h_buf.values[i]);
    n += sprintf(resbuf + n, "\r\n");
    if (p_res->entity)
        n += sprintf(resbuf + n, "%s", p_res->entity);
    return n;
}

int main(int argc, char *argv[])
{
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
        (struct sockaddr *) &cli_addr, 
        &clilen)) == -1) 
        errno_exit("accepting failed.");
    create_session("127.0.0.1", inet_ntoa(cli_addr.sin_addr));

    struct request req;
    struct response res;
    char buffer[BUFFER_SIZE], resbuf[BUFFER_SIZE];
    int n, cnt = 6, ret = 0;
    while (cnt --) {
        if (ret == 1) {
            printf("=== cnt: %d ===\n", cnt);
            
            continue;
        }
        CLEAR_BUF(buffer);
        if ((n = recv(newsockfd, buffer, BUFFER_SIZE - 1, 0)) == -1)
            errno_exit("reading from socket failed");
        //if (cnt == 1)
        //    printf("{%s}\n", buffer);
        parse_request(buffer, n, &req);
        fprint_request(stdout, &req);

        ret = process_request(&req, &res);
        n = create_response_message(resbuf, &res);
        if ((n = send(newsockfd, resbuf, n, 0)) == -1)
            errno_exit("writing to socket failed");
        printf("====== response ======\n%s\n", resbuf);
    }

    close(newsockfd);
    close(sockfd);
    return 0; 
}