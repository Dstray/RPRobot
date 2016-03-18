/*
 *  V4L2 video capture
 *  Author: Dstray
 *  common/common.c
 */

#include "common.h"

static const char digit[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static char itoabuf[64];

char* itoa(int value, char* str, int base) {
    assert(base > 1 && base <= 36);
    int idx = 0, i = 0;
    if (value < 0) {
        value = -value;
        str[i++] = '-';
    }
    while (value) {
        itoabuf[idx++] = digit[value % base];
        value /= base;
    }
    while (idx > 0)
        str[i++] = itoabuf[--idx];
    str[i] = '\0';
    return str;
}

void errno_exit(const char* s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void exception_exit(const char* s1, const char* s2) {
    fprintf(stderr, "Exception: %s %s\n", s1, s2);
    exit(EXIT_FAILURE);
}