gcc -o capture common/common.c video/imgproc.c video/capture.c
gcc -o rtspserver common/common.c video/imgproc.c rtsp/rtsp.c rtsp/header.c rtsp/rtp/rtp.c rtsp/method.c rtsp/server.c
#gcc -o rtspclient common/common.c rtsp/rtsp.c rtsp/header.c rtsp/method.c rtsp/client.c