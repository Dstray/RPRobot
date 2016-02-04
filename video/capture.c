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

struct buffer* init_read_io() {}
struct buffer* init_mmap_io() {}
struct buffer* init_userptr_io() {}

void check_dev_cap(int fd, const char* dev_name, enum io_method io) {
    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        if (errno == EINVAL)
            exception_exit(dev_name, "is not V4L2 device");
        else
            errno_exit("VIDIOC_QUERYCAP");
    }
    fprintf(stdout, "Device Capabilities: 0x%08x\n", cap.capabilities);
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        exception_exit(dev_name, "is not video capture device");
    switch (io) {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE))
            exception_exit(dev_name, "does not support read i/o");
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING))
            exception_exit(dev_name, "does not support streaming i/o");
        break;
    }
}

void fprint_cropcap(FILE* stream, struct v4l2_cropcap* pcap) {
    fprintf(stream, "Video cropping and scaling abilities:\n");
    fprintf(stream,
        "  bounds: { left:%d, top:%d, width:%d, height:%d }\n",
        pcap->bounds.left, pcap->bounds.top,
        pcap->bounds.width, pcap->bounds.height);
    fprintf(stream,
        "  defrect: { left:%d, top:%d, width:%d, height:%d }\n",
        pcap->defrect.left, pcap->defrect.top,
        pcap->defrect.width, pcap->defrect.height);
    fprintf(stream, "  pixelaspect: %d:%d\n",
        pcap->pixelaspect.numerator,
        pcap->pixelaspect.denominator);
}

void set_cropping_rect(int fd, const char* dev_name) {
    // Check the cropping limits
    struct v4l2_cropcap cropcap;
    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_CROPCAP, &cropcap) == 0) {
        fprint_cropcap(stdout, &cropcap);
    } else {
        if (errno == EINVAL)
            exception_report("struct v4l2_cropcap type", "is invalid");
        else
            errno_report("VIDIOC_CROPCAP");
    }
    // Set the current cropping rectangle
    struct v4l2_crop crop;
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;
    if (xioctl(fd, VIDIOC_S_CROP, &crop) == -1) {
        if (errno == EINVAL)
            exception_report(dev_name, "does not support cropping");
        else
            errno_report("VIDIOC_S_CROP");
    }
}

void fprint_image_format(FILE* stream, struct v4l2_pix_format* pix) {
    unsigned pf = pix->pixelformat;
    fprintf(stream, "Information of image format:\n");
    fprintf(stream, "  width: %d, height: %d\n", pix->width, pix->height);
    fprintf(stream, "  pixel format: %c%c%c%c\n",
        pf >> 0, pf >> 8, pf >> 16, pf >> 24);
    fprintf(stream, "  field order: %d\n", pix->field);
    fprintf(stream, "  bytes per line: %u\n", pix->bytesperline);
    fprintf(stream, "  image size: %u bytes\n", pix->sizeimage);
    fprintf(stream, "  color space: %u\n", pix->colorspace);
}

int list_supported_image_formats(int fd) {
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtdesc.index = 0;
    unsigned pf;
    fprintf(stdout, "Enumerate image formats:\n");
    fprintf(stdout, "  index   pixelformat flags   description\n");
    while (1) {
        if (xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == -1) {
            if (errno == EINVAL)
                break;
            else
                errno_exit("VIDIOC_ENUM_FMT");
        }
        pf = fmtdesc.pixelformat;
        fprintf(stdout, "  %02d      ", fmtdesc.index ++);
        fprintf(stdout, "%c%c%c%c        ",
            pf >> 0, pf >> 8, pf >> 16, pf >> 24);
        fprintf(stdout, "0x%04x  ", fmtdesc.flags);
        fprintf(stdout, "%s\n", fmtdesc.description);
    }
}

void set_image_format(int fd, struct v4l2_format* pfmt) {
    CLEAR(*pfmt);
    pfmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_G_FMT, pfmt) == -1)
        errno_exit("VIDIOC_G_FMT");
    struct v4l2_pix_format* pix = &(pfmt->fmt.pix);
    if (0) {
        pix->width       = 640;
        pix->height      = 480;
        pix->pixelformat = V4L2_PIX_FMT_YUYV;
        pix->field       = V4L2_FIELD_INTERLACED;
        if (xioctl(fd, VIDIOC_S_FMT, pfmt) == -1)
            errno_exit("VIDIOC_S_FMT");
    }
    fprint_image_format(stdout, pix);
}

void init_device(int fd, const char* dev_name, enum io_method io) {
    // Check device capabilities
    check_dev_cap(fd, dev_name, io);
    // Set the cropping rectangle
    set_cropping_rect(fd, dev_name);
    // Set the image format
    struct v4l2_format fmt;
    list_supported_image_formats(fd);
    set_image_format(fd, &fmt);
    // Allocate buffers
    switch (io) {
    case IO_METHOD_READ:
        init_read_io();
        break;
    case IO_METHOD_MMAP:
        init_mmap_io();
        break;
    case IO_METHOD_USERPTR:
        init_userptr_io();
        break;
    }
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