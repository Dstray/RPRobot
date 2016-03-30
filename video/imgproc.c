#include "imgproc.h"

int jpeg_check_dht(unsigned char* imgbuf, size_t* imgsz) {
    unsigned char* ptr = imgbuf;
    unsigned char* bufend = imgbuf + *imgsz;
    while (ptr < bufend) {
        if ((*ptr++) != 0xFF)
            continue;
        unsigned char identifier = *ptr;
        if (identifier == JPEG_EOI) { //EOI
            ptr = imgbuf + 1; //After SOI
            break;
        } else if ((identifier & 0xF0) != JPEG_SOF0) { //Not SOF
            ptr += ((int)ptr[1] << 8 | ptr[2]) + 1;
            continue;
        }
        if (identifier == JPEG_DHT || (identifier & 0x0F) >= 0x08) //DHT or SOF
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

int jpeg_parse_dqt(unsigned char* start, struct jpeg_frame* p_jframe) {
    int len = (int)start[0] << 8 | start[1];
    unsigned char p = start[2] >> 4;
    if (!(p_jframe->lqt)) {
        p_jframe->qt_size = (p + 1) * 64;
        p_jframe->lqt = &start[3];
    } else if (!(p_jframe->cqt)) {
        p_jframe->cqt = &start[3];
    }
    return len;
}

int jpeg_parse_dri(unsigned char* start, struct jpeg_frame* p_jframe) {
    int len = (int)start[0] << 8 | start[1];
    p_jframe->restart_interval = (int)start[2] << 8 | start[3];
    return len;
}

int jpeg_parse_sof(unsigned char* start, struct jpeg_frame* p_jframe) {
    int len = (int)start[0] << 8 | start[1];
    p_jframe->height = (int)start[3] << 8 | start[4];
    p_jframe->width = (int)start[5] << 8 | start[6];
    return len;
}

int jpeg_parse_sos(unsigned char* start, struct jpeg_frame* p_jframe) {
    int len = (int)start[0] << 8 | start[1];
    return len;
}

int jpeg_get_frame_details(unsigned char* imgbuf, int imgsz,
        struct jpeg_frame* p_jframe) {
    unsigned char* ptr = imgbuf;
    unsigned char* bufend = imgbuf + imgsz;
    unsigned char identifier;
    while (ptr < bufend) {
        if ((*ptr++) != 0xFF) {
            //printf("Assertion Failed at imgproc.c:67\n");
            continue;
        }
        identifier = *ptr;
        if (identifier == JPEG_DQT) {
            ptr += jpeg_parse_dqt(++ptr, p_jframe);
        } else if (identifier == JPEG_DRI) {
            ptr += jpeg_parse_dri(++ptr, p_jframe);
        } else if (identifier == JPEG_SOF0) {
            ptr += jpeg_parse_sof(++ptr, p_jframe);
        } else if (identifier == JPEG_SOS) {
            ptr += jpeg_parse_sos(++ptr, p_jframe) - 2;
            ptr[0] = 0xFF;
            ptr[1] = JPEG_SOI;
            break;
        } else if (identifier == JPEG_SOI) {
            ptr += 1;
        } else {
            ptr += ((int)ptr[1] << 8 | ptr[2]) + 1;
        }
    }
    if (ptr >= bufend)
        ;//printf("Assertion Failed at imgproc.c:87\n");
    p_jframe->scan_data = ptr;
    p_jframe->data_size = bufend - ptr;
    return 0;
}
