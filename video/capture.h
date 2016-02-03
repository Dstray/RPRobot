/*
 *  V4L2 video capture
 *  Author: Dstray
 *  Video capture part for the RPRobot
 */

#ifndef __CAPTURE_H__
#define __CAPTURE_H__ 1

#include "../common/exception.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

enum io_method {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR
};

extern int open_device(const char* dev_name);
extern void init_device(int fd, const char* dev_name, enum io_method io);
extern void close_device(int fd);

#endif /* capture.h */