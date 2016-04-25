/*
 *  OSS audio recording
 *  Author: Dstray
 *  Audio recording section for the RPRobot
 */

#ifndef __RECORDING_H__
#define __RECORDING_H__ 1

#include "../common/common.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>

#define WAV_BUFFER_SIZE 0x0400

extern int oss_open_device(const char *name);
extern void oss_init_device(int fd, int buf_size);
extern struct buffer* oss_record(int fd);
extern void oss_close_device(int fd);

#endif /* recording.h */