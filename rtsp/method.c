#include "method.h"

void process_method_unsupported(void* p_req, void* p_res) {}

void process_method_describe(void* p_req, void* p_res) {}

void process_method_options(void* p_req, void* p_res) {
    p_req = (struct request*)p_req;
    p_res = (struct response*)p_res;
    
}

void process_method_play(void* p_req, void* p_res) {}
void process_method_setup(void* p_req, void* p_res) {}
void process_method_teardown(void* p_req, void* p_res) {}