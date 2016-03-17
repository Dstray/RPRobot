#include "rtsp.h"

struct header_set {
	unsigned type;
	struct header* headers;
	int size;
};

static struct header_set h_set[] = {
	{ HEADER_TYPE_GENERAL, general_headers, SIZEOF(general_headers) },
	{ HEADER_TYPE_REQUEST, request_headers, SIZEOF(request_headers) },
	{ HEADER_TYPE_RESPONSE, response_headers, SIZEOF(response_headers) },
	{ HEADER_TYPE_ENTITY, entity_headers, SIZEOF(entity_headers) },
	{ HEADER_TYPE_EXTENSION, extension_headers, SIZEOF(extension_headers) }
};

struct header* _get_header(const char* name, struct header* headers, int size) {
	int s = 0, g = size, m, r;
	while (s < g) {
		m = (s + g) / 2;
		r = strcmp(name, headers[m].name);
		if (r < 0)
			g = m;
		else if (r > 0)
			s = m + 1;
		else
			return &headers[m];
	}
	return NULL;
}

struct header* get_header(const char* name, unsigned type) {
	struct header* h = NULL;
	int i;
	for (i = 0; i != SIZEOF(h_set); i++) {
		if (!(h_set[i].type & type))
			continue;
		h = _get_header(name, h_set[i].headers, h_set[i].size);
		if (h)
			break;
	}
	return h;
}