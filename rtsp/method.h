#ifndef __METHOD_H__
#define __METHOD_H__ 1

#include "../common/common.h"
#include "header.h"

extern int process_method_unsupported(void*, void*);
extern int process_method_describe(void*, void*);
extern int process_method_options(void*, void*);
extern int process_method_play(void*, void*);
extern int process_method_setup(void*, void*);
extern int process_method_teardown(void*, void*);

#endif /* method.h */