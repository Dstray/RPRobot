#ifndef __HEADER_H__
#define __HEADER_H__ 1

#include "../common/common.h"

struct status {
    int code;
    const char* reason_phrase;
};

struct request_line {
    char method[15];
    char req_uri[40];
    char version[10];
};

struct status_line {
    char version[10];
    struct status* p_status;
};

struct header_buffers { //TODO
    int num;
    struct header* fields[10];
    char* values[10];
};

struct request {
    struct request_line req_line;
    struct header_buffers h_bufs;
};

struct response {
    struct status_line sta_line;
    struct header_buffers h_bufs;
    char* entity;
};

struct method {
    const char* name;
    void (*func)(void*, void*); // (struct request*, struct response*)
};

struct header {
    const char* name;
    void (*func)(void*, void*, void*); // (void*, struct response* | struct request*)
};

#define HEADER_TYPE_GENERAL     0x01
#define HEADER_TYPE_REQUEST     0x02
#define HEADER_TYPE_RESPONSE    0x04
#define HEADER_TYPE_ENTITY      0x08
#define HEADER_TYPE_EXTENSION   0x10
#define HEADER_TYPE_REQ         0x1B
#define HEADER_TYPE_RES         0x1D

extern void process_header_default(void*, void*, void*);

// general header

// request header
extern void process_header_accept(void*, void*, void*);

// response header
extern void process_header_public(void*, void*, void*);

// entity header
extern void process_header_content_length(void*, void*, void*);
extern void process_header_content_type(void*, void*, void*);

// extension header
extern void process_header_cseq(void*, void*, void*);

static const char* const supported_methods[] = {
    "DESCRIBE",
    "OPTIONS",
    "PLAY",
    "SETUP",
    "TEARDOWN"
};

#endif /* header.h */