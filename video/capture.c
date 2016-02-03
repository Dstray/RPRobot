#include <getopt.h>
#include "capture.h"

static char*            dev_name = "/dev/video0";
static enum io_method   io = IO_METHOD_MMAP;
static int              frame_count = 10;

static int xioctl(int fd, int request, void* argp) {
    int r;
    do {
        r = ioctl(fd, request, argp);
    } while (r == -1 && errno == EINTR); // Interrupted function call
    return r;
}

void init_device(int fd, const char* dev_name, enum io_method io) {
    struct v4l2_capability cap;
    // Check capabilities
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) { // Query device capabilities
        if (errno == EINVAL)
            exception_exit(dev_name, "is not V4L2 device");
        else
            errno_exit("VIDIOC_QUERYCAP");
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        exception_exit(dev_name, "is not video capture device");

}

int open_device(const char* dev_name) {
    struct stat st; // Information of a file
    if (stat(dev_name, &st) == -1) {
        fprintf(stderr, "Cannot identify '%s': Error %d, %s\n",
            dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (!S_ISCHR(st.st_mode))
        exception_exit(dev_name, "is not character device.");
    int fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (fd == -1) {
        fprintf(stderr, "Cannot open '%s': Error %d, %s\n",
            dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

void close_device(int fd) {
    if (close(fd) == -1)
        errno_exit("Device Close");
}

static void print_usage(FILE* fp, int argc, char** argv) {
    fprintf(fp,
        "Usage: %s [options]\n\n"
        "Version 1.0\n"
        "Options:\n"
        "-d | --device name   Video device name [%s]\n"
        "-h | --help          Print this message\n"
        "-m | --mmap          Use memory mapped buffers [default]\n"
        "-r | --read          Use read() calls\n"
        "-u | --userp         Use application allocated buffers\n"
        //"-o | --output        Outputs stream to stdout\n"
        //"-f | --format        Force format to 640x480 YUYV\n"
        "-c | --count         Number of frames to grab [%i]\n"
        "",
        argv[0], dev_name, frame_count);
}

static const char short_options[] = "d:hmruofc:";

static const struct option long_options[] = {
//  { name,     has_arg,            flag, val }
    { "device", required_argument,  NULL, 'd' },
    { "help",   no_argument,        NULL, 'h' },
    { "mmap",   no_argument,        NULL, 'm' },
    { "read",   no_argument,        NULL, 'r' },
    { "userp",  no_argument,        NULL, 'u' },
    //{ "output", no_argument,        NULL, 'o' },
    //{ "format", no_argument,        NULL, 'f' },
    { "count",  required_argument,  NULL, 'c' },
    { 0, 0, 0, 0 } // last element
};

int main(int argc, char** argv) {
    int c, ind;
    while ((c = getopt_long(argc, argv, short_options, long_options, &ind)) != -1) {
        switch (c) {
        case 0: //long_options flag not NULL
            break;
        case 'd':
            dev_name = optarg;
            break;
        case 'h':
            print_usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);
        case 'm':
            io = IO_METHOD_MMAP;
            break;
        case 'r':
            io = IO_METHOD_READ;
            break;
        case 'u':
            io = IO_METHOD_USERPTR;
            break;/*
        case 'o':
            out_buf++;
            break;
        case 'f':
            force_format++;
            break;*/
        case 'c':
            errno = 0;
            frame_count = strtol(optarg, NULL, 0);
            if (errno)
                errno_exit(optarg);
            if (frame_count <= 0)
                exception_exit("Invalid frame number", optarg);
            break;
        default:
            print_usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    int fd = open_device(dev_name);
    init_device(fd, dev_name, io);
    close_device(fd);

    return 0;
}