#ifndef __METHOD_H__
#define __METHOD_H__ 1

#include "../common/common.h"
#include "header.h"
#include "rtp/rtp.h"
#include <time.h>

#define NTP_TIME_OFFSET 2208988800l

unsigned short seq;
unsigned int rtptime;
unsigned int ssrc;

extern int process_method_unsupported(void*, void*);
extern int process_method_describe(void*, void*);
extern int process_method_options(void*, void*);
extern int process_method_play(void*, void*);
extern int process_method_setup(void*, void*);
extern int process_method_teardown(void*, void*);

typedef struct {
    unsigned type_specific:8;
    unsigned fragment_offset:24;
    unsigned type:8;
    unsigned q:8;
    unsigned width:8;
    unsigned height:8;
} jpeg_hdr_t;

#endif /* method.h */