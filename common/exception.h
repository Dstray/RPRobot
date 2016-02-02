/*
 *  V4L2 video capture
 *  Author: Dstray
 *  Common exception part for the RPRobot
 */

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__ 1

#include "common.h"
#include <errno.h>

extern static void errno_exit(const char* s);
extern static void exception_exit(const char* s1, const char* s2);

#endif /* exception.h */