#include <getopt.h>
#include "capture.h"

static char*            dev_name = "/dev/video0";
static enum io_method   io = IO_METHOD_MMAP;
struct buffer*          buffers;
static unsigned         n_buffers = 0;
static int              frame_count = 10;
struct v4lconvert_data* cvt_data; //\\

static int xioctl(int fd, int request, void* argp) {
    int r;
    do {
        r = ioctl(fd, request, argp);
    } while (r == -1 && errno == EINTR); // Interrupted function call
    return r;
}

void process_image(unsigned char* rdata, int size) {
    FILE* fp = fopen("frame.txt", "w");
    int i, j;
    for (i = 0; i != 614400;) {
        for (j = 0; j != 1280; j++, i++)
            fprintf(fp, "%02x", rdata[i]);
        fprintf(fp, "\n");
    }
    fclose(fp);
    
}

void fprint_timecode(FILE* stream, struct v4l2_timecode* ptcode) {
    fprintf(stream, "  timecode:\n");
    fprintf(stream, "    type:       %d\n", ptcode->type);
    fprintf(stream, "    flags:      0x%08x\n", ptcode->flags);
    fprintf(stream, "    frames:     %d\n", ptcode->frames);
    fprintf(stream, "    seconds:    %d\n", ptcode->seconds);
    fprintf(stream, "    minutes:    %d\n", ptcode->minutes);
    fprintf(stream, "    hours:      %d\n", ptcode->hours);
}

void fprint_buffer_status(FILE* stream, struct v4l2_buffer* pbuf) {
    fprintf(stream, "Buffer %d:\n", pbuf->index);
    fprintf(stream, "  bytesused:    %d\n", pbuf->bytesused);
    fprintf(stream, "  flags:        0x%08x\n", pbuf->flags);
    fprintf(stream, "  field:        %d\n", pbuf->field);
    fprintf(stream, "  timestamp:    %dus\n", pbuf->timestamp.tv_usec);
    //fprint_timecode(stream, &(pbuf->timecode));
    fprintf(stream, "  sequence:     %d\n", pbuf->sequence);
    fprintf(stream, "  memory:       %d\n", pbuf->memory);
    fprintf(stream, "  length:       %d\n", pbuf->length);
}

int read_frame(int fd, enum io_method io) {
    struct v4l2_buffer buf;
    int i = 0;
    switch (io) {
    case IO_METHOD_READ:
        if (read(fd, buffers[0].start, buffers[0].length) == -1) {
            switch (errno) {
            case EAGAIN:
            case EINTR:
                return -1;
            case EIO:
            default:
                errno_exit("read");
            }
        }
        process_image(buffers[0].start, buffers[0].length);
        break;
    case IO_METHOD_MMAP:
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            switch (errno) {
            case EAGAIN:
            case EPIPE:
                return -1;
            case EIO:
            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }
        assert(buf.index < n_buffers);
        process_image(buffers[buf.index].start, buf.bytesused);
        fprint_buffer_status(stdout, &buf);
        i = buf.index;
        if (xioctl(fd, VIDIOC_QBUF, &buf) == -1)
            errno_exit("VIDIOC_QBUF");
        break;
    case IO_METHOD_USERPTR:
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            switch (errno) {
            case EAGAIN:
            case EPIPE:
                return -1;
            case EIO:
            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }
        for (i = 0; i < n_buffers; ++i)
            if (buf.m.userptr == (unsigned long)buffers[i].start
                && buf.length == buffers[i].length)
                break;
        assert(i < n_buffers);
        process_image((void *)buf.m.userptr, buf.bytesused);
        fprint_buffer_status(stdout, &buf);
        i = buf.index;
        if (xioctl(fd, VIDIOC_QBUF, &buf) == -1)
            errno_exit("VIDIOC_QBUF");
        break;
    }
    return i;
}

struct buffer* capture(int fd, enum io_method io) {
    fd_set fds;
    struct timeval tv;
    int retval;
    do {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 2; // Timeout
        tv.tv_usec = 0;
        retval = select(fd + 1, &fds, NULL, NULL, &tv);
        if (retval == -1) {
            if (errno == EINTR)
                continue;
            errno_exit("select");
        } else if (retval == 0) {
            exception_exit("Waiting for capturing", "timeout");
        }
    } while ((retval = read_frame(fd, io)) == -1);
    return &(buffers[retval]);
}

void stop_capturing(int fd, enum io_method io) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    switch (io) {
    case IO_METHOD_READ:
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
        break;
    }
}

void start_capturing(int fd, enum io_method io) {
    enum v4l2_memory mem_t;
    switch (io) {
    case IO_METHOD_READ:
        return; // Nothing to do
    case IO_METHOD_MMAP:
        mem_t = V4L2_MEMORY_MMAP;
        break;
    case IO_METHOD_USERPTR:
        mem_t = V4L2_MEMORY_USERPTR;
        break;
    }
    int i;
    struct v4l2_buffer buf;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (i = 0; i != n_buffers; i++) {
        CLEAR(buf);
        buf.type = type;
        buf.memory = mem_t;
        buf.index = i;
        if (io == IO_METHOD_USERPTR) {
            buf.m.userptr = (unsigned long)buffers[i].start;
            buf.length = buffers[i].length;
        }
        if (xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
    }
    if (xioctl(fd, VIDIOC_STREAMON, &type) == -1)
        errno_exit("VIDIOC_STREAMON");
}

void init_read_io(int buf_size) {
    n_buffers = 1; // multile buffers using VIDIOC_S_PARM
    buffers = calloc(n_buffers, sizeof(*buffers));
    if (!buffers)
        exception_exit("Failed to alloc space for buffers", "");
    buffers[0].length = buf_size;
    buffers[0].start = malloc(buf_size);
    if (!buffers[0].start)
        exception_exit("Failed to alloc space for buffers", "");
}

void init_stream_io(int fd, const char* dev_name,
    struct v4l2_requestbuffers* preq,
    int n_bufs, enum v4l2_memory mem_t) {
    CLEAR(*preq);
    preq->count = n_bufs;
    preq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    preq->memory = mem_t;
    if (xioctl(fd, VIDIOC_REQBUFS, preq) == -1) {
        if (EINVAL == errno)
            exception_exit(dev_name,
                "does not support this specified streaming i/o");
        else
            errno_exit("VIDIOC_REQBUFS");
    }
    if (mem_t == V4L2_MEMORY_MMAP && preq->count < 2)
        exception_exit("Insufficient buffer memory on", dev_name);

    buffers = calloc(preq->count, sizeof(*buffers));
    if (!buffers)
        exception_exit("Failed to alloc space for buffers", "");
}

void init_mmap_io(int fd, const char* dev_name, int n_bufs) {
    // Request buffers
    struct v4l2_requestbuffers req;
    init_stream_io(fd, dev_name, &req, n_bufs, V4L2_MEMORY_MMAP);
    // Map buffers
    struct v4l2_buffer buf;
    for (n_buffers = 0; n_buffers != req.count; n_buffers++) {
        // Query buffer status
        CLEAR(buf);
        buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.index   = n_buffers;
        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
            errno_exit("VIDIOC_QUERYBUF");
        //fprint_buffer_status(stdout, &buf);
        // Map
        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap(
            NULL, // start anywhere
            buf.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, // recommended
            fd, buf.m.offset
        );
        if (buffers[n_buffers].start == MAP_FAILED)
            errno_exit("mmap");
    }
}

void init_userptr_io(int fd, const char* dev_name,
    int buf_size, int n_bufs) {
    // Request buffers
    struct v4l2_requestbuffers req;
    init_stream_io(fd, dev_name, &req, n_bufs, V4L2_MEMORY_USERPTR);
    // Allocate memory space for buffers
    for (n_buffers = 0; n_buffers < n_bufs; ++n_buffers) {
        buffers[n_buffers].length = buf_size;
        buffers[n_buffers].start = malloc(buf_size);
        if (!buffers[n_buffers].start)
            exception_exit("Failed to alloc space for buffers", "");
    }
}

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
    fprintf(stream, "  color space: %u %u\n", pix->colorspace, V4L2_COLORSPACE_JPEG);
    fprintf(stream, "  extended fields: %u\n", pix->priv == V4L2_PIX_FMT_PRIV_MAGIC ? 1 : 0);
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
    if (1) {
        pix->width       = 960;
        pix->height      = 720;
        pix->pixelformat = V4L2_PIX_FMT_MJPEG;
        pix->field       = V4L2_FIELD_INTERLACED;
    }
    fprint_image_format(stdout, pix);
}

void init_device(int fd, const char* dev_name,
    enum io_method io, int n_bufs) {
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
        init_read_io(fmt.fmt.pix.sizeimage);
        break;
    case IO_METHOD_MMAP:
        init_mmap_io(fd, dev_name, n_bufs);
        break;
    case IO_METHOD_USERPTR:
        init_userptr_io(fd, dev_name,
            fmt.fmt.pix.sizeimage, n_bufs);
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

    /*cvt_data = v4lconvert_create(fd);
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtdesc.index = 0;
    unsigned pf;
    fprintf(stdout, "Enumerate dest image formats:\n");
    fprintf(stdout, "  index   pixelformat flags   description\n");
    while (1) {
        if (v4lconvert_enum_fmt(cvt_data, &fmtdesc) == -1) {
            if (errno == EINVAL)
                break;
            else
                errno_exit("v4lconvert_enum_fmt");
        }
        pf = fmtdesc.pixelformat;
        fprintf(stdout, "  %02d      ", fmtdesc.index ++);
        fprintf(stdout, "%c%c%c%c        ",
            pf >> 0, pf >> 8, pf >> 16, pf >> 24);
        fprintf(stdout, "0x%04x  ", fmtdesc.flags);
        fprintf(stdout, "%s\n", fmtdesc.description);
    }*/

    return fd;
}

void close_device(int fd, enum io_method io) {
    /*v4lconvert_destroy(cvt_data);*/
    // Clear buffers
    int i;
    switch (io) {
    case IO_METHOD_MMAP:
        for (i = 0; i != n_buffers; i++)
            if (munmap(buffers[i].start, buffers[i].length) == -1)
                errno_report("munmap");
        break;
    case IO_METHOD_READ:
    case IO_METHOD_USERPTR:
        for (i = 0; i != n_buffers; i++)
            free(buffers[i].start);
        break;
    }
    free(buffers);
    // Close the device
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
    init_device(fd, dev_name, io, 4);
    start_capturing(fd, io);
    while (frame_count--)
        capture(fd, io);
    stop_capturing(fd, io);
    close_device(fd, io);

    return 0;
}