#include "imgproc.h"

int check_huffman_table(unsigned char* imgbuf, int* imgsz) {
	unsigned char* ptr = imgbuf;
	unsigned char* bufend = imgbuf + *imgsz;
	while (ptr < bufend) {
		if ((*ptr++) != 0xFF)
			continue;
		unsigned char identifier = *ptr;
		if ((identifier & 0xF0) != 0xC0 || (identifier & 0x0F) >= 0x08) // Not SOF
			continue;
		if (identifier == 0xC4) //DHT
			return 0;
		if (identifier == 0xD9) { //EOI
			ptr = imgbuf + 2; // After SOI
			break;
		}
		ptr++;
		break;
	}
}