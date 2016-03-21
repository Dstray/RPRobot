#ifndef __RTSP_H__
#define __RTSP_H__ 1

#include "../common/common.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "header.h"
#include "method.h"

#define RTSP_VERSION "RTSP/1.0"

enum state {
    INIT,
    READY,
    PLAYING,
    RECORDING
};

static struct status response_status[] = {
    { 100, "Continue" },
    { 200, "OK" },
    { 201, "Created" },
    { 250, "Low on Storage Space" },
    { 300, "Multiple Choices" },
    { 301, "Moved Permanently" },
    { 302, "Moved Temporarily" },
    { 303, "See Other" },
    { 305, "Use Proxy" },
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 402, "Payment Required" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 405, "Method Not Allowed" },
    { 406, "Not Acceptable" },
    { 407, "Proxy Authentication Required" },
    { 408, "Request Timeout" },
    { 410, "Gone" },
    { 411, "Length Required" },
    { 412, "Precondition Failed" },
    { 413, "Request Entity Too Large" },
    { 414, "Request-URI Too Long" },
    { 415, "Unsupported Media Type" },
    { 451, "Invalid parameter" },
    { 452, "Illegal Conference Identifier" },
    { 453, "Not Enough Bandwidth" },
    { 454, "Session Not Found" },
    { 455, "Method Not Valid In This State" },
    { 456, "Header Field Not Valid" },
    { 457, "Invalid Range" },
    { 458, "Parameter Is Read-Only" },
    { 459, "Aggregate Operation Not Allowed" },
    { 460, "Only Aggregate Operation Allowed" },
    { 461, "Unsupported Transport" },
    { 462, "Destination Unreachable" },
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Unavailable" },
    { 504, "Gateway Timeout" },
    { 505, "RTSP Version Not Supported" },
    { 551, "Option not support" }
};

static struct method methods[] = {
    { "DESCRIBE", process_method_describe },
    { "OPTIONS", process_method_options },
    { "PLAY", process_method_play },
    { "SETUP", process_method_setup },
    { "TEARDOWN", process_method_teardown }
};

// IN DICTIONARY ORDER
static struct header general_headers[] = {
    { "Cache-Control", process_header_default },            //SETUP
    { "Connection", process_header_default },               //SETUP
    { "Date", process_header_default },
    { "Via", process_header_default }
};

static struct header request_headers[] = {
    { "Accept", process_header_accept },                   //entity
    { "Accept-Encoding", process_header_default },          //entity
    { "Accept-Language", process_header_default },
    { "Authorization", process_header_default },
    { "From", process_header_default },
    { "If-Modified-Since", process_header_default },        //DESCRIBE, SETUP
    { "Range", process_header_default },                    //PLAY, PAUSE, RECORD
    { "Referer", process_header_default },
    { "User-Agent", process_header_default }
};

static struct header response_headers[] = {
    { "Location", process_header_default },///
    { "Proxy-Authenticate", process_header_default },
    { "Public", process_header_public },///
    { "Retry-After", process_header_default },
    { "Server", process_header_default },
    { "Vary", process_header_default },///
    { "WWW-Authenticate", process_header_default }
};

static struct header entity_headers[] = {
    { "Allow", process_header_default },
    { "Content-Base", process_header_default },             //entity
    { "Content-Encoding", process_header_default },         //SET_PARAMETER | (DESCRIBE, ANNOUNCE)
    { "Content-Language", process_header_default },         //DESCRIBE, ANNOUNCE
    { "Content-Length", process_header_default },           //(SET_PARAMETER, ANNOUNCE) | entity
    { "Content-Location", process_header_default },         //entity
    { "Content-Type", process_header_default },             //(SET_PARAMETER, ANNOUNCE) | entity
    { "Expires", process_header_default },                  //DESCRIBE, ANNOUNCE
    { "Last-Modified", process_header_default }             //entity
};

static struct header extension_headers[] = {
    { "Bandwidth", process_header_default },
    { "Blocksize", process_header_default },                //all but OPTIONS, TEARDOWN
    { "Conference", process_header_default },               //SETUP
    { "CSeq", process_header_cseq },
    { "Host", process_header_default },///
    { "If-Match", process_header_default },///
    { "Proxy-Require", process_header_default },
    { "Require", process_header_default },
    { "RTP-Info", process_header_default },                 //PLAY
    { "Scale", process_header_default },                    //PLAY, RECORD
    { "Session", process_header_default },                  //all but SETUP, OPTIONS
    { "Speed", process_header_default },                    //PLAY
    { "Timestamp", process_header_default },///
    { "Transport", process_header_default },                //SETUP
    { "Unsupported", process_header_default },
};

// type: HEADER_TYPE_
extern struct header* get_header(const char* name, unsigned type);
extern struct status* get_status(int code);

#endif /* rtsp.h */