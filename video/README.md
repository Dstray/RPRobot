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
  * bufs: pointer to struct _buffer_ type buffers allocated through _init_device_.  
* referenced functions
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
  * bufs: pointer to unallocated struct _buffer_ type buffers.  

**check_dev_cap**  
Query device capabilities and check according to I/O method.  
* parameters
  * fd: file descripter returned from _open_device_.
  * dev_name: name of the device.
  * io: specified I/O method.
* referenced functions
  * [ioctl VIDIOC_QUERYCAP](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html) @[sys/ioctl.h]&[linux/videodev2.h]  

**set_cropping_rect**  
Check the cropping limits and set the rectangle.  
* parameters
  * fd: file descripter returned from _open_device_.
  * dev_name: name of the device.
* referenced functions
  * [ioctl VIDIOC_CROPCAP](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-cropcap.html)  
  * [ioctl VIDIOC_S_CROP](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-crop.html)  

**list_supported_image_formats**  
List all supported image formats by the device.  
* parameters
  * fd: file descripter returned from _open_device_.
* return value
  * Number of supported image formats.
* referenced functions
  * [ioctl VIDIOC_ENUM_FMT](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html)  

**set_image_format**  
Set the image format exchanged between drivers and applications.  
* parameters
  * fd: file descripter returned from _open_device_.
  * pfmt: pointer of an struct _v4l2_format_ type object.  
* referenced functions
  * [ioctl VIDIOC_G_FMT, VIDIOC_S_FMT](http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html)  


## Packages
libv4l  
v4l-utils  
kmod-video-uvc

## Devices
a UVC Webcam: /dev/videoN

## References
[Video Capture Example](http://linuxtv.org/downloads/v4l-dvb-apis/capture-example.html)
: LINUX MEDIA INFRASTRUCTURE API