#ifndef __RTP_UDP_PUSH_H
#define __RTP_UDP_PUSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5602
#define RTP_HEADER_SIZE 12
#define UDP_PACKET_SIZE 1472
#define RTP_PACKET_SIZE UDP_PACKET_SIZE - RTP_HEADER_SIZE

typedef struct
{
    int sock;
    struct sockaddr_in addr;
    int seq_num;
    uint8_t rtp_header[RTP_HEADER_SIZE];
} Udp_Connection_S;

int rtp_udp_push(Udp_Connection_S *conn, uint8_t *frame_data, size_t frame_size, uint32_t frame_pts);
int rtp_udp_push_init(Udp_Connection_S *conn);
int rtp_udp_push_deinit(Udp_Connection_S *conn);

#endif //__RTP_UDP_PUSH_H