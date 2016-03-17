#ifndef __RTSP_H__
#define __RTSP_H__ 1

#include "../common/exception.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "method.h"

#define RTSP_VERSION "RTSP/1.0"

#define SIZEOF(o) (sizeof (o)) / (sizeof *(o))

//^header.h

//header.h$

enum state {
	INIT,
	READY,
	PLAYING,
	RECORDING
};

struct status {
	int code;
	const char* reason_phrase;
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

struct request_line {
    char method[15];
    char req_uri[40];
    char version[10];
};

struct status_line {
    char* version;
    struct status* p_status;
};

struct message_headers { //TODO
    int num;
    char* fields[10];
    char* values[10];
};

struct request {
	struct request_line req_line;
	struct message_headers m_headers;
};

struct response {
	struct status_line sta_line;
	struct message_headers m_headers;
};

struct method {
	const char* name;
	void (*func)(void*, void*); // (struct request*, struct response*)
};

static struct method methods[] = {
	{ "DESCRIBE", process_method_describe },
	{ "OPTIONS", process_method_options },
	{ "PLAY", process_method_play },
	{ "SETUP", process_method_setup },
	{ "TEARDOWN", process_method_teardown }
};

#endif /* rtsp.h */