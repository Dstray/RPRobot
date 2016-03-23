#include "header.h"

#define TMP_BUFFER_SIZE 0x0400

static char tmpbuf[TMP_BUFFER_SIZE];
static int b_idx = 0;
static int proc_req; // 1 on process request; 2 on process response

extern struct status* get_status(int code);

void add_header(struct header_buffer* p_hbuf, struct header* p_hdr, char* val) {
    p_hbuf->fields[p_hbuf->num] = p_hdr;
    p_hbuf->values[p_hbuf->num] = val;
    p_hbuf->num++;
    assert(p_hbuf->num <= 10);
}

void add_header_str(struct header_buffer* p_hbuf, struct header* p_hdr, char* val) {
    char* value = tmpbuf + b_idx;
    b_idx += sprintf(value, "%s", val) + 1;
    add_header(p_hbuf, p_hdr, value);
}

void add_header_int(struct header_buffer* p_hbuf, struct header* p_hdr, long val) {
    char* value = tmpbuf + b_idx;
    b_idx += sprintf(value, "%ld", val) + 1;
    add_header(p_hbuf, p_hdr, value);
}

void process_header(void* vp_req, void* vp_res) {
    struct request* p_req = (struct request*)vp_req;
    struct response* p_res = (struct response*)vp_res;
    b_idx = 0;
    proc_req = !strlen(p_res->sta_line.version);
    assert(proc_req ^ !strlen(p_req->req_line.version));

    struct header_buffer* p_hbuf;
    void* vp_msg;
    if (proc_req) {
        p_res->sta_line.p_status = get_status(200);
        p_res->h_buf.num = 0;
        p_res->entity = NULL;
        p_hbuf = &(p_req->h_buf);
        vp_msg = vp_res;
    } else {
        p_req->h_buf.num = 0;
        p_hbuf = &(p_res->h_buf);
        vp_msg = vp_req;
    }

    int i;
    for (i = 0; i != p_hbuf->num; i++)
        p_hbuf->fields[i]->func(p_hbuf->fields[i], p_hbuf->values[i], vp_msg);
}

void process_header_default(void* vp_hdr, void* vp_val, void* vp_msg) {}

// general header

// request header
void process_header_accept(void* vp_hdr, void* vp_val, void* vp_msg) {
    assert(proc_req);
    char *val = (char*) vp_val, *type = val;
    int i = 0;
    while (val[i] != '\0') {
        if (val[i] == ',' || val[i] == ';') { //TODO
            val[i] = '\0';
            if (!strcmp(type, "application/sdp")) {
                ((struct response*)vp_msg)->entity = "application/sdp";
                return;
            }
            if (val[i] == ';')
                while (val[++i] != ',' && val[i] != '\0') ;
            while (val[++i] == ' ') ;
            type = &val[i--];
        }
        i++;
    }
    if (!strcmp(type, "application/sdp"))
        ((struct response*)vp_msg)->entity = "application/sdp";
    else
        ((struct response*)vp_msg)->sta_line.p_status = get_status(406);
}

// response header
void process_header_public(void* vp_hdr, void* vp_val, void* vp_msg) {
    assert(proc_req); //TODO
    struct header_buffer* p_hbuf = &((struct response*)vp_msg)->h_buf;
    add_header(p_hbuf, (struct header*)vp_hdr, tmpbuf + b_idx);

    int n_methods = SIZEOF(supported_methods), i;
    for (i = 0; i != n_methods; i++) {
        if (i != 0)
            b_idx += sprintf(tmpbuf + b_idx, ", ");
        b_idx += sprintf(tmpbuf + b_idx, "%s", supported_methods[i]);
    }
    tmpbuf[b_idx++] = '\0';
}

// entity header
void process_header_content_length(void* vp_hdr, void* vp_val, void* vp_msg) {}

void process_header_content_type(void* vp_hdr, void* vp_val, void* vp_msg) {}

// extension header
void process_header_cseq(void* vp_hdr, void* vp_val, void* vp_msg) {
    struct header_buffer* p_hbuf;
    char* value;
    if (proc_req) {
        p_hbuf = &((struct response*)vp_msg)->h_buf;
        value = (char*)vp_val;
    } else {
        p_hbuf = &((struct request*)vp_msg)->h_buf;
        value = tmpbuf + b_idx;
        b_idx += sprintf(value, "%d", atoi((char*)vp_val) + 1) + 1;
    }
    add_header(p_hbuf, (struct header*)vp_hdr, value);
}

void process_header_transport(void* vp_hdr, void* vp_val, void* vp_msg) {
    struct header_buffer* p_hbuf;
    char *c = (char*)vp_val, *value = c, *trans = c, *cli_port = NULL;
    char* param_k;
    if (proc_req) {
        int udp = 1, i = 0, allowed = 1, finished = 0, sign_cli = 0;
        while (*c && !finished) {
            switch (*c) {
            case ';':
                *c = '\0';
                if (i == 0) {
                    if (strlen(value) == 11) {
                        if (strcmp("TCP", c - 3))
                            assert(!strcmp("UDP", c - 3));
                        else
                            udp = 0;
                        *(c - 4) = '\0';
                        assert(!strcmp("RTP/AVP", value));
                        *(c - 4) = '/';
                    } else
                        assert(!strcmp("RTP/AVP", value));
                } else if (i == 1) {
                    if (strcmp("multicast", value))
                        assert(!strcmp("unicast", value));
                    else {
                        while ((*++c) && (*c) != ',') ;
                        c--;
                        i = allowed = 0;
                        break;
                    }
                } else {
                    if (sign_cli) {
                        cli_port = tmpbuf + b_idx;
                        b_idx += sprintf(cli_port, "%s", value) + 1;
                        sign_cli = 0;
                    }
                }
                i++;
                value = c + 1;
                *c = ';';
                break;
            case ',':
                if (allowed) {
                    finished = 1;
                    *c = '\0';
                } else {
                    while (*++c == ' ') ;
                    trans = value = c--;
                    allowed = 1;
                }
                break;
            case '=':
                *c = '\0';
                param_k = tmpbuf + b_idx;
                sprintf(param_k, "%s", value);
                if (!strcmp(param_k, "client_port"))
                    sign_cli = 1;
                *c = '=';
                value = c + 1;
                break;
            }
            c++;
        }
        if (sign_cli) {
            cli_port = tmpbuf + b_idx;
            b_idx += sprintf(cli_port, "%s", value) + 1;
        }
        if (allowed) {
            c = tmpbuf + b_idx;
            b_idx += sprintf(c, "%s", trans);
            printf("client_port: %s\n", cli_port);
            if (cli_port)
                b_idx += sprintf(tmpbuf + b_idx, ";server_port=%d", 6403);
            b_idx++;
            add_header(&((struct response*)vp_msg)->h_buf, (struct header*)vp_hdr, c);
        } else ; //TODO
    }
}