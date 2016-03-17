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

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define CLEAR_BUF(b) memset(b, 0, sizeof b)

struct buffer {
    void   *start;
    size_t  length;
};

#endif /* common.h */
