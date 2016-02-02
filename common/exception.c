/*
 *  V4L2 video capture
 *  Author: Dstray
 *  common/exception.c
 */

#include "exception.h"

static void errno_exit(const char* s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static void exception_exit(const char* s1, const char* s2) {
    fprintf(stderr, "Exception: %s %s\n", s1, s2);
    exit(EXIT_FAILURE);
}