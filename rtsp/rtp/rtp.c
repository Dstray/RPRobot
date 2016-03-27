/*
 * rtp.c  --  general RTP utils
 */
#include "rtp.h"

/* Procedure rtp_send_jframe:
 *
 *  Arguments:
 *    start_seq: The sequence number for the first packet of the current
 *               frame.
 *    ts: RTP timestamp for the current frame
 *    ssrc: RTP SSRC value
 *    jpeg_data: Huffman encoded JPEG scan data
 *    len: Length of the JPEG scan data
 *    type: The value the RTP/JPEG type field should be set to
 *    typespec: The value the RTP/JPEG type-specific field should be set
 *              to
 *    width: The width in pixels of the JPEG image
 *    height: The height in pixels of the JPEG image
 *    dri: The number of MCUs between restart markers (or 0 if there
 *         are no restart markers in the data
 *    q: The Q factor of the data, to be specified using the Independent
 *       JPEG group's algorithm if 1 <= q <= 99, specified explicitly
 *       with lqt and cqt if q >= 128, or undefined otherwise.
 *    lqt: The quantization table for the luminance channel if q >= 128
 *    cqt: The quantization table for the chrominance channels if
 *         q >= 128
 *    ic: The interleaved channel specified on RTSP when transmitting
 *        via TCP.
 *
 *  Return value:
 *    the sequence number to be sent for the first packet of the next
 *    frame.
 *
 * The following are assumed to be defined:
 *
 * RTP_PACKET_SIZE_MAX                 - The size of the outgoing packet
 * send_packet(u_int8 *data, int len)  - Sends the packet to the network
 */

u_int16 rtp_send_jframe(int sock_fd, u_int16 start_seq, u_int32 ts, u_int32 ssrc,
                        struct jpeg_frame* p_jframe, u_int8 q, int ic) {
    rtp_hdr_t rtphdr;
    struct jpeghdr jpghdr;
    struct jpeghdr_rst rsthdr;
    struct jpeghdr_qtable qtblhdr;
    u_int8 packet_buf[RTP_PACKET_SIZE_MAX];
    u_int8 *ptr;
    int bytes_left = p_jframe->data_size;
    int qt_len = p_jframe->qt_size, dri = p_jframe->restart_interval;
    int pkt_len, data_len, offset = 0;
    int rtp_hdr_len = ic < 0 ? RTP_HDR_SIZE : RTP_HDR_SIZE + 4;

    /* Initialize RTP header
     */
    rtphdr.version = 2;
    rtphdr.p = 0;
    rtphdr.x = 0;
    rtphdr.cc = 0;
    rtphdr.m = 0;
    rtphdr.pt = RTP_PT_JPEG;
    rtphdr.seq = htons(start_seq);
    rtphdr.ts = htonl(ts);
    rtphdr.ssrc = ssrc;printf("%02x\n", *(u_int8*)(&rtphdr));

    /* Initialize JPEG header
     */
    jpghdr.tspec = RTP_JPEG_TYPESPEC;
    jpghdr.off = 0;
    jpghdr.type = RTP_JPEG_TYPE | ((dri != 0) ? RTP_JPEG_RESTART : 0);
    jpghdr.q = q;
    jpghdr.width = p_jframe->width / 8;
    jpghdr.height = p_jframe->height / 8;

    /* Initialize DRI header
     */
    if (dri != 0) {
        rsthdr.dri = dri;
        rsthdr.f = 1;        /* This code does not align RIs */
        rsthdr.l = 1;
        rsthdr.count = 0x3fff;
    }

    /* Initialize quantization table header
     */
    if (q >= 128) {
        qtblhdr.mbz = 0;
        qtblhdr.precision = 0; /* This code uses 8 bit tables only */
        qtblhdr.length = 2 * qt_len;  /* 2 64-byte tables */
    }

    while (bytes_left > 0) {
        ptr = packet_buf + rtp_hdr_len;
        memcpy(ptr, &jpghdr, sizeof(jpghdr));
        ptr += sizeof(jpghdr);

        if (dri != 0) {
            memcpy(ptr, &rsthdr, sizeof(rsthdr));
            ptr += sizeof(rsthdr);
        }

        if (q >= 128 && jpghdr.off == 0) {
            memcpy(ptr, &qtblhdr, sizeof(qtblhdr));
            ptr += sizeof(qtblhdr);
            memcpy(ptr, p_jframe->lqt, qt_len);
            ptr += qt_len;
            memcpy(ptr, p_jframe->cqt, qt_len);
            ptr += qt_len;
        }

        data_len = RTP_PACKET_SIZE_MAX - (ptr - packet_buf);
        if (data_len >= bytes_left) {
            data_len = bytes_left;
            rtphdr.m = 1;
        }

        pkt_len = (ptr - packet_buf) + data_len;
        if (ic < 0)
            memcpy(packet_buf, &rtphdr, RTP_HDR_SIZE);
        else {
            packet_buf[0] = '$';
            packet_buf[1] = (u_int8)ic;
            *(u_int16*)(&packet_buf[2]) = htons((u_int16)pkt_len - 4);
            memcpy(packet_buf + 4, &rtphdr, RTP_HDR_SIZE);
        }
        memcpy(ptr, p_jframe->scan_data + jpghdr.off, data_len);

        send(sock_fd, packet_buf, pkt_len, 0);

        jpghdr.off += data_len;
        bytes_left -= data_len;
        rtphdr.seq = htons(++start_seq);
    }
    return rtphdr.seq;
}