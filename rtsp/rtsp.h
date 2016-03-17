#include "../common/exception.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

enum state {
	INIT,
	READY,
	PLAYING,
	RECORDING
};

struct request_line {
    char method[15];
    char req_uri[40];
    char version[10];
};

struct status_line {
    char verion[10];
    int code;
    char reason_phrase[40];
};

struct message_headers {
    int num;
    char* keys[10];
    char* values[10];
};