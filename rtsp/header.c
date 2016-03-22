#include "header.h"

#define TMP_BUFFER_SIZE 0x0400

static char tmpbuf[TMP_BUFFER_SIZE];
static int b_idx = 0;
static int proc_req; // 1 on process request; 2 on process response

extern struct status* get_status(int code);

void add_header(struct header_buffers* p_hbufs, struct header* p_hdr, char* val) {
    p_hbufs->fields[p_hbufs->num] = p_hdr;
    p_hbufs->values[p_hbufs->num] = val;
    p_hbufs->num++;
    assert(p_hbufs->num <= 10);
}

void process_header(void* vp_req, void* vp_res) {
    struct request* p_req = (struct request*)vp_req;
    struct response* p_res = (struct response*)vp_res;
    b_idx = 0;
    proc_req = !strlen(p_res->sta_line.version);
    assert(proc_req ^ !strlen(p_req->req_line.version));

    struct header_buffers* p_hbufs;
    void* vp_msg;
    if (proc_req) {
        p_res->sta_line.p_status = get_status(200);
        p_res->h_bufs.num = 0;
        p_res->entity = NULL;
        p_hbufs = &(p_req->h_bufs);
        vp_msg = vp_res;
    } else {
        p_req->h_bufs.num = 0;
        p_hbufs = &(p_res->h_bufs);
        vp_msg = vp_req;
    }

    int i;
    for (i = 0; i != p_hbufs->num; i++)
        p_hbufs->fields[i]->func(p_hbufs->fields[i], p_hbufs->values[i], vp_msg);
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
    struct header_buffers* p_hbufs = &((struct response*)vp_msg)->h_bufs;
    add_header(p_hbufs, (struct header*)vp_hdr, tmpbuf + b_idx);

    int n_methods = SIZEOF(supported_methods), i;
    for (i = 0; i != n_methods; i++) {
        if (i != 0)
            b_idx += sprintf(tmpbuf + b_idx, ", ");
        b_idx += sprintf(tmpbuf + b_idx, "%s", supported_methods[i]);
    }
    tmpbuf[b_idx++] = '\0';
}

// entity header
void process_header_content_length(void* vp_hdr, void* vp_val, void* vp_msg) {
    struct header_buffers* p_hbufs;
    if (proc_req)
        p_hbufs = &((struct response*)vp_msg)->h_bufs;
    else
        p_hbufs = &((struct request*)vp_msg)->h_bufs;
    char* value = itoa(*((int*)vp_val), tmpbuf + b_idx, 10);
    b_idx += strlen(value) + 1;
    add_header(p_hbufs, (struct header*)vp_hdr, value);
}

void process_header_content_type(void* vp_hdr, void* vp_val, void* vp_msg) {
    struct header_buffers* p_hbufs;
    char* type;
    if (proc_req) {
        type = tmpbuf + b_idx;
        b_idx += sprintf(type, "%s", (char*)vp_val) + 1;
        p_hbufs = &((struct response*)vp_msg)->h_bufs;
        add_header(p_hbufs, (struct header*)vp_hdr, type);
    } else {
        type = (char*)vp_val;
        //TODO
    }
}

// extension header
void process_header_cseq(void* vp_hdr, void* vp_val, void* vp_msg) {
    struct header_buffers* p_hbufs;
    char* value;
    if (proc_req) {
        p_hbufs = &((struct response*)vp_msg)->h_bufs;
        value = (char*)vp_val;
    } else {
        p_hbufs = &((struct request*)vp_msg)->h_bufs;
        value = itoa(atoi((char*)vp_val) + 1, tmpbuf + b_idx, 10);
        b_idx += strlen(value) + 1;
    }
    add_header(p_hbufs, (struct header*)vp_hdr, value);
}