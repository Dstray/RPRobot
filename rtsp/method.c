#include "method.h"

extern struct header* get_header(const char* name, unsigned type);

void process_method_unsupported(void* p_req, void* p_res) {
    process_header(p_req, p_res);
}

void process_method_describe(void* p_req, void* p_res) {
    process_header(p_req, p_res);
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
}

void process_method_teardown(void* p_req, void* p_res) {
    process_header(p_req, p_res);
}