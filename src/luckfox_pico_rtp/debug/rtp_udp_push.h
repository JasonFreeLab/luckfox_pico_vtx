#ifndef __RTP_UDP_PUSH_H
#define __RTP_UDP_PUSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 5602                                           // 定义 UDP 端口号
#define RTP_HEADER_SIZE 12                                  // RTP 头部大小
#define UDP_PACKET_SIZE 1472                                // UDP 数据包的最大大小
#define RTP_PACKET_SIZE (UDP_PACKET_SIZE - RTP_HEADER_SIZE) // 可用于 RTP 数据的大小

// 定义用于 RTP UDP 连接的结构体
typedef struct
{
    int sock;                            // 套接字描述符
    struct sockaddr_in addr;             // 存储远端地址
    uint64_t seq_num;                    // RTP 序列号
    uint8_t rtp_header[RTP_HEADER_SIZE]; // RTP 头部数据
} Udp_Connection_S;

/**
 * @brief 推送 RTP 数据通过 UDP 连接
 *
 * @param conn 指向 Udp_Connection_S 结构的指针，包含连接信息
 * @param frame_data 需要发送的帧数据的指针
 * @param frame_size 帧数据的大小
 * @param frame_pts 帧的时间戳
 * @return int 返回 0 表示成功，其他值表示失败
 */
int rtp_udp_push(Udp_Connection_S *conn, uint8_t *frame_data, size_t frame_size, uint32_t frame_pts);

/**
 * @brief 推送 RTP 数据通过 UDP 连接
 *
 * @param conn 指向 Udp_Connection_S 结构的指针，包含连接信息
 * @param frame_data 需要发送的帧数据的指针
 * @param frame_size 帧数据的大小
 * @param frame_pts 帧的时间戳
 * @return int 返回 0 表示成功，其他值表示失败
 */
int rtp_udp_push_new(Udp_Connection_S *conn, uint8_t *frame_data, size_t frame_size, uint32_t frame_pts);

/**
 * @brief 初始化 RTP UDP 推送连接
 *
 * @param conn 指向 Udp_Connection_S 结构的指针，包含连接信息
 * @return int 返回 0 表示成功，其他值表示失败
 */
int rtp_udp_push_init(Udp_Connection_S *conn);

/**
 * @brief 释放 RTP UDP 推送连接资源
 *
 * @param conn 指向 Udp_Connection_S 结构的指针，包含连接信息
 * @return int 返回 0 表示成功，其他值表示失败
 */
int rtp_udp_push_deinit(Udp_Connection_S *conn);

#endif //__RTP_UDP_PUSH_H