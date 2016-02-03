# Video

## Capture
Video capture via V4L2.

### Options
[getopt_long](http://linux.die.net/man/3/getopt_long) @**getopt.h**

### I/O Method
**[Read/Write](http://linuxtv.org/downloads/v4l-dvb-apis/io.html#rw)**
* IO_METHOD_READ

**[Streaming I/O](http://linuxtv.org/downloads/v4l-dvb-apis/mmap.html)**
* IO_METHOD_MMAP
* IO_METHOD_USERPTR

### Open & Close
**open_device**
Check if it's a character device and open it.  
* parameters
  * dev_name: name of the device.
* return value
  * file descriptor.
* referenced functions
  * [stat](http://linux.die.net/man/2/stat) @**sys/stat.h**  
  * [open](http://linuxtv.org/downloads/v4l-dvb-apis/func-open.html) @**fcntl.h**

**close_device**
* parameters
  * fd: file descripter returned from _open_device_.
* referenced functions
  * [close](http://linuxtv.org/downloads/v4l-dvb-apis/func-close.html) @**unistd.h**  

### Initialize
**init_device**
* parameters
  * fd: file descripter returned from _open_device_.
  * dev_name: name of the device.
  * io: specified I/O method.

## Packages
libv4l  
v4l-utils
kmod-video-uvc

## Devices
a UVC Webcam: /dev/videoN

## References
[Video Capture Example](http://linuxtv.org/downloads/v4l-dvb-apis/capture-example.html)
: LINUX MEDIA INFRASTRUCTURE API