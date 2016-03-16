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

