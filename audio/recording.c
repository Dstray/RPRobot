#include "recording.h"

static char*            dev_name = "/dev/dsp";
static struct buffer*   buffers;
static unsigned         n_buffers = 0;
static int              sample_rate = 48000;

static int
oss_open_device(const char *name) {
	int fd;

    if ((fd = open(name, O_RDONLY, 0)) == -1) {
        perror(name);
        exit(EXIT_FAILURE);
    }

    return fd;
}

static void
oss_init_device(int fd, int buf_size) {
    int tmp = AFMT_S16_NE;      /* Native 16 bits */
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
        errno_exit("SNDCTL_DSP_SETFMT");

    if (tmp != AFMT_S16_NE)
        exception_exit(dev_name, "doesn't support the 16 bit sample format.");

    tmp = 1;
    if (ioctl(fd, SNDCTL_DSP_CHANNELS, &tmp) == -1)
        errno_exit("SNDCTL_DSP_CHANNELS");

    if (tmp != 1)
        exception_exit(dev_name, "doesn't support mono mode.");

    if (ioctl (fd, SNDCTL_DSP_SPEED, &sample_rate) == -1)
        errno_exit("SNDCTL_DSP_SPEED");

    printf("Sample rate: %dHz", sample_rate);

    n_buffers = 1;
    buffers = calloc(n_buffers, sizeof(*buffers));
    if (!buffers)
        exception_exit("Failed to alloc space for buffers", "");
    buffers[0].length = buf_size;
    buffers[0].start = malloc(buf_size);
    if (!buffers[0].start)
        exception_exit("Failed to alloc space for buffers", "");
}

static struct buffer* 
oss_record(int fd) {
    void *buf = buffers[0].start;
    if ((buffers[0].length = read(fd, buf, sizeof buf)) == -1) {
        errno_report("Audio read");
        return;
    }
}

void oss_close_device(fd) {
    int i;
    for (i = 0; i != n_buffers; i++)
        free(buffers[i].start);
    free(buffers);
    // Close the device
    if (close(fd) == -1)
        errno_exit("Device Close");
}

int
main (int argc, char *argv[])
{
    if (argc > 1)
        dev_name = argv[1];
    int fd = oss_open_device(dev_name);
    oss_init_device(fd, 0x0400);

    struct buffer* wavbuf;
    int level, i, len;
    while (1) {
        wavbuf = oss_record(fd);
        short *buf = (short *)wavbuf->start;

        level = 0;
        len = wavbuf->length / 2;
        for (i = 0; i < len; i++) {
            int v = buf[i];

            if (v < 0)
                v = -v;         /* abs */

            if (v > level)
                level = v;
        }
        level = (level + 1) / 1024;

        for (i = 0; i < level; i++)
            printf ("*");
        for (i = level; i < 32; i++)
            printf (".");
        printf ("\r");
        fflush (stdout);
    }

    return 0;
}