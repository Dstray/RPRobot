gcc -o capture common/exception.c video/imgproc.c video/capture.c
gcc -o rtspserver common/exception.c rtsp/method.c rtsp/server.c
gcc -o rtspclient common/exception.c rtsp/method.c rtsp/client.c