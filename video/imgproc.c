#include "imgproc.h"

int check_dht(unsigned char* imgbuf, int* imgsz) {
	unsigned char* ptr = imgbuf;
	unsigned char* bufend = imgbuf + *imgsz;
	while (ptr < bufend) {
		if ((*ptr++) != 0xFF)
			continue;
		unsigned char identifier = *ptr;
		if (identifier == 0xD9) { //EOI
			ptr = imgbuf + 1; //After SOI
			break;
		} else if ((identifier & 0xF0) != 0xC0) { //Not SOF
			ptr += (ptr[1] << 8 | ptr[2]);
			continue;
		}
		if (identifier == 0xC4 || (identifier & 0x0F) >= 0x08) //DHT or SOF
			return 0;
		break; //SOF0
	}
	if (++ptr > bufend)
		ptr = imgbuf + 2;
	int ht_size = sizeof huffman_table;
	memmove(ptr + ht_size, ptr, bufend - ptr);
	memcpy(ptr, huffman_table, ht_size);
	*imgsz += ht_size;
	return 0;
}