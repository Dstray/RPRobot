/*
 *  V4L2 video capture
 *  Author: Dstray
 *  Common included *.h files
 */

#ifndef __COMMOM_H__
#define __COMMOM_H__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define CLEAR_BUF(b) memset(b, 0, sizeof b)

#define SIZEOF(o) (sizeof (o)) / (sizeof *(o))

struct buffer {
    void   *start;
    size_t  length;
};

extern char* itoa(int value, char* str, int base);

#define errno_report(s) fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno))
#define exception_report(s1, s2) fprintf(stderr, "Exception: %s %s\n", s1, s2)

extern void errno_exit(const char* s);
extern void exception_exit(const char* s1, const char* s2);

#endif /* common.h */
