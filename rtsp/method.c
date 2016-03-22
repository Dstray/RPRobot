#include "method.h"
#include <time.h>

#define ENTITY_BUFFER_SIZE 0x0400
#define NTP_TIME_OFFSET 2208988800l
#define VIDEO_PORT 4588
#define PAYLOAD_TYPE_JPEG 26 // https://tools.ietf.org/html/rfc3551#page-33

static char entity_buf[ENTITY_BUFFER_SIZE];
static int b_idx;
static long session_id, version;
static const char* ser_addr;
static const char* cli_addr;

void create_session(const char* sa, const char* ca) {
    session_id = (long)time(NULL) + NTP_TIME_OFFSET;
    ser_addr = sa;
    cli_addr = ca;
}

extern struct header* get_header(const char* name, unsigned type);

void process_method_unsupported(void* p_req, void* p_res) {
    process_header(p_req, p_res);
}

void process_method_describe(void* p_req, void* p_res) {
    process_header(p_req, p_res);
    char *type = ((struct response*)p_res)->entity;
    struct header* p_h;
    struct header_buffer* p_hbuf = &((struct response*)p_res)->h_buf;
    if (type) {
        p_h = get_header("Content-Type", HEADER_TYPE_ENTITY);
        add_header_str(p_hbuf, p_h, type);

        b_idx = 0;
        version = (long)time(NULL) + NTP_TIME_OFFSET;
        b_idx += sprintf(entity_buf + b_idx, "v=0\r\n");
        b_idx += sprintf(entity_buf + b_idx, "o=- %ld %ld IN IP4 %s\r\n",
            session_id, version, cli_addr);
        b_idx += sprintf(entity_buf + b_idx, "s=RTSP Session\r\n");
        b_idx += sprintf(entity_buf + b_idx, "c=IN IP4 %s\r\n", ser_addr);
        b_idx += sprintf(entity_buf + b_idx, "t=%ld %ld\r\n", session_id, session_id + 1800);
        b_idx += sprintf(entity_buf + b_idx, "m=video %d RTP/AVP %d\r\n", VIDEO_PORT, PAYLOAD_TYPE_JPEG);
        ((struct response*)p_res)->entity = entity_buf;

        p_h = get_header("Content-Length", HEADER_TYPE_ENTITY);
        add_header_int(p_hbuf, p_h, b_idx);
    }
}

void process_method_options(void* p_req, void* p_res) {
	process_header(p_req, p_res);
    struct header* p_h = get_header("Public", HEADER_TYPE_RESPONSE);
    p_h->func((void*)p_h, NULL, p_res);
}

void process_method_play(void* p_req, void* p_res) {
    process_header(p_req, p_res);
}

void process_method_setup(void* p_req, void* p_res) {
    process_header(p_req, p_res);
    struct header_buffer* p_hbuf = &((struct response*)p_res)->h_buf;
    struct header* p_h = get_header("Session", HEADER_TYPE_EXTENSION);
    add_header_int(p_hbuf, p_h, session_id);
    p_h = get_header("Date", HEADER_TYPE_GENERAL);
    time_t curtime = time(NULL);
    add_header_str(p_hbuf, p_h, asctime(localtime(&curtime)));
}

void process_method_teardown(void* p_req, void* p_res) {
    process_header(p_req, p_res);
}