/*
 *  V4L2 video capture
 *  Author: Dstray
 *  Common exception part for the RPRobot
 */

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__ 1

#include "common.h"
#include <errno.h>

#define errno_report(s) fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno))
#define exception_report(s) fprintf(stderr, "Exception: %s\n")

extern void errno_exit(const char* s);
extern void exception_exit(const char* s1, const char* s2);

#endif /* exception.h */