#include "recording.h"

static int
open_audio_device (const char *name, int sample_rate) {
	int tmp, fd;

    if ((fd = open(name, O_RDONLY, 0)) == -1) {
        perror(name);
        exit(EXIT_FAILURE);
    }

    tmp = AFMT_S16_NE;		/* Native 16 bits */
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
        errno_exit("SNDCTL_DSP_SETFMT");

    if (tmp != AFMT_S16_NE)
        exception_exit(name, "doesn't support the 16 bit sample format.");

    tmp = 1;
    if (ioctl(fd, SNDCTL_DSP_CHANNELS, &tmp) == -1)
        errno_exit("SNDCTL_DSP_CHANNELS");

    if (tmp != 1)
        exception_exit(name, "doesn't support mono mode.");

    if (ioctl (fd, SNDCTL_DSP_SPEED, &sample_rate) == -1)
        errno_exit("SNDCTL_DSP_SPEED");

    return fd;
}

void
process_input (void) {
    short buffer[1024];

    int len, i, level;

    if ((l = read(fd, buffer, sizeof buffer)) == -1) {
        errno_report("Audio read");
        return;
    }

    len /= 2;

    level = 0;

    for (i = 0; i < len; i++)
    {
        int v = buffer[i];

        if (v < 0)
            v = -v;			/* abs */

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

int
main (int argc, char *argv[])
{
    char *name_in = "/dev/dsp";
    if (argc > 1)
        name_in = argv[1];
    int fd = open_audio_device(name_in, 48000);

    while (1)
        process_input(fd);

    close(fd);

    return 0;
}