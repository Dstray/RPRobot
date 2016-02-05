# Video

## Capture
Video capture via V4L2.

### Options
[getopt_long](http://linux.die.net/man/3/getopt_long) @[getopt.h]

### I/O Method
**Read/Write**
* [IO_METHOD_READ](http://linuxtv.org/downloads/v4l-dvb-apis/io.html#rw)

**Streaming I/O**
* [IO_METHOD_MMAP](http://linuxtv.org/downloads/v4l-dvb-apis/mmap.html)
* [IO_METHOD_USERPTR](http://linuxtv.org/downloads/v4l-dvb-apis/userp.html)

### Open & Close
**open_device**  
Check if it's a character device and open it.  
* parameters
  * dev_name: name of the device.
* return value
  * file descriptor.
* referenced functions
  * [stat](http://linux.die.net/man/2/stat) @[sys/stat.h]  
  * [open](http://linuxtv.org/downloads/v4l-dvb-apis/func-open.html) @[fcntl.h]

**close_device**  
Clear buffers and close the device.
* parameters
  * fd: file descripter returned from _open_device_.  
* referenced functions
  * [munmap](http://linuxtv.org/downloads/v4l-dvb-apis/func-munmap.html) @[sys/mman.h]
  * [close](http://linuxtv.org/downloads/v4l-dvb-apis/func-close.html) @[unistd.h]  

### Initialize
**init_device**  
1. Check device capabilities;  
2. Set the cropping rectangle;  
3. Set the image format;  
4. Initialize buffers.  
* parameters
  * fd: file descripter returned from _open_device_.
  * dev_name: name of the device.
  * io: specified I/O method.

**check_dev_cap**  
Query device capabilities and check according to I/O method.  
* parameters
  * fd: file descripter of the device.
  * dev_name: name of the device.
  * io: specified I/O method.
* referenced functions
  * [ioctl VIDIOC_QUERYCAP](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html) @[sys/ioctl.h]&[linux/videodev2.h]  

**set_cropping_rect**  
Check the cropping limits and set the rectangle.  
* parameters
  * fd: file descripter of the device.
  * dev_name: name of the device.
* referenced functions
  * [ioctl VIDIOC_CROPCAP](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-cropcap.html)  
  * [ioctl VIDIOC_S_CROP](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-crop.html)  

**list_supported_image_formats**  
List all supported image formats by the device.  
* parameters
  * fd: file descripter of the device.
* return value
  * Number of supported image formats.
* referenced functions
  * [ioctl VIDIOC_ENUM_FMT](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html)  

**set_image_format**  
Set the image format exchanged between drivers and applications.  
* parameters
  * fd: file descripter of the device.
  * pfmt: pointer of an struct [_v4l2_format_](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html#v4l2-format) type object.  
* referenced functions
  * [ioctl VIDIOC_G_FMT, VIDIOC_S_FMT](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html)  

**init_read_io**
* parameters
  * buf_size: size of each buffer in bytes.  

**init_mmap_io**  
Initiate Memory Mapping I/O:  
Request several buffers (each for one image capture);  
Map all the allocated buffers from device memory space to application memory space.
* parameters
  * fd: file descripter of the device.
  * dev_name: name of the device.  
* referenced functions
  * [ioctl VIDIOC_REQBUFS](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html)
  * [mmap](http://linuxtv.org/downloads/v4l-dvb-apis/func-mmap.html) @[sys/mman.h]

**init_userptr_io**
* parameters
  * fd: file descripter of the device.
  * dev_name: name of the device.
  * buf_size: size of each buffer in bytes.  
* referenced functions
  * [ioctl VIDIOC_REQBUFS](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html)  

### Capturing
**start_capturing**  
Streaming I/O:  
Enqueue previously requested buffers in the driver's incoming queue;  
Start capture process during streaming I/O.  
Read/Write I/O:  
Nothing to do.
* parameters
  * fd: file descripter returned from _open_device_.
  * io: specified I/O method.  
* referenced functions
  * [ioctl VIDIOC_QBUF](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-qbuf.html)
  * [ioctl VIDIOC_STREAMON](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-streamon.html)

**stop_capturing**  
Streaming I/O:  
Stop capture process during streaming I/O.  
Read/Write I/O:  
Nothing to do.
* parameters
  * fd: file descripter returned from _open_device_.
  * io: specified I/O method.
* referenced functions
  * [ioctl VIDIOC_STREAMOFF](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-streamon.html)


### Image Format

<buffer status></buffer>

## Packages
libv4l  
v4l-utils  
kmod-video-uvc

## Devices
a UVC Webcam: /dev/videoN

## References
[Video Capture Example](http://linuxtv.org/downloads/v4l-dvb-apis/capture-example.html)
: LINUX MEDIA INFRASTRUCTURE API