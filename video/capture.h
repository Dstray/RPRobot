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

extern int open_device(const char* dev_name);
extern void init_device(int fd, const char* dev_name);
extern void close_device(int fd);

#endif /* capture.h */