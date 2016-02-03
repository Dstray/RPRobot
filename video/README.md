# Video

## Capture
Video capture via V4L2.

### Options
[getopt_long](http://linux.die.net/man/3/getopt_long) @**getopt.h**

### Open & Close
**open_device**
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

## Packages
libv4l  
v4l-utils

## Devices
a UVC Webcam: /dev/videoN

## References
[Video Capture Example](http://linuxtv.org/downloads/v4l-dvb-apis/capture-example.html)
: LINUX MEDIA INFRASTRUCTURE API