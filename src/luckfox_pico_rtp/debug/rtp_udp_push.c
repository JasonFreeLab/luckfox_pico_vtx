#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "rtp_udp_push.h"

uint8_t *frame_data_end; // 指向尚未发送的帧数据的缓冲区
size_t frame_size_end = 0; // 当前还未发送的帧数据的大小

/**
 * @brief 初始化UDP连接
 *
 * 此函数初始化给定的UDP连接结构，包括创建socket
 * 并设置目标地址和端口。
 *
 * @param conn 指向Udp_Connection_S结构的指针，用于存储连接信息
 * @return 返回0表示成功，-1表示失败
 */
int init_udp_connection(Udp_Connection_S *conn)
{
    if (conn == NULL)
        return -1;

    // 创建UDP socket
    conn->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (conn->sock < 0)
    {
        perror("socket");
        return -1;
    }

    // 设置目标地址和端口
    conn->addr.sin_family = AF_INET; // 使用IPv4地址
    conn->addr.sin_port = htons(PORT); // 设置端口号（使用网络字节顺序）
    conn->addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 设置目标IP地址为本地回环地址

    return 0;
}

/**
 * @brief 初始化RTP头
 *
 * 此函数初始化给定的UDP连接中的RTP头，设置序列号和时间戳。
 *
 * @param conn 指向Udp_Connection_S结构的指针，用于存储连接信息
 * @return 返回0表示成功，-1表示失败
 */
int init_rtp_header(Udp_Connection_S *conn)
{
    if (conn == NULL)
        return -1;

    conn->seq_num = 0;

    static uint32_t timestamp = 0; // 可在每次发送时更新
    uint32_t ssrc = rand(); // 随机生成 SSRC，避免冲突

    memset(conn->rtp_header, 0, RTP_HEADER_SIZE);

    // 填充RTP头信息
    conn->rtp_header[0] = 0x80; // RTP版本2，填充0，扩展标志0
    conn->rtp_header[1] = 96; // 动态负载类型
    conn->rtp_header[2] = (conn->seq_num >> 8) & 0xFF; // 序列号高位
    conn->rtp_header[3] = (conn->seq_num) & 0xFF;      // 序列号低位
    // 填充时间戳
    conn->rtp_header[4] = (timestamp >> 24) & 0xFF; // 时间戳高位
    conn->rtp_header[5] = (timestamp >> 16) & 0xFF; // 时间戳中位
    conn->rtp_header[6] = (timestamp >> 8) & 0xFF;  // 时间戳低位
    conn->rtp_header[7] = (timestamp) & 0xFF;       // 时间戳低位
    // 填充SSRC
    conn->rtp_header[8] = (ssrc >> 24) & 0xFF; // SSRC高位
    conn->rtp_header[9] = (ssrc >> 16) & 0xFF; // SSRC中位
    conn->rtp_header[10] = (ssrc >> 8) & 0xFF; // SSRC低位
    conn->rtp_header[11] = (ssrc) & 0xFF;      // SSRC低位

    return 0;
}

/**
 * @brief 构建RTP包
 *
 * 此函数根据当前的序列号和给定的帧时间戳构建RTP包头。
 *
 * @param conn 指向Udp_Connection_S结构的指针，用于存储连接信息
 * @param frame_pts 当前帧的时间戳
 */
void build_rtp_packet(Udp_Connection_S *conn, uint32_t frame_pts)
{
    conn->rtp_header[2] = (conn->seq_num >> 8) & 0xFF; // 序列号高位
    conn->rtp_header[3] = (conn->seq_num) & 0xFF;      // 序列号低位
    // 填充时间戳
    conn->rtp_header[4] = (frame_pts >> 24) & 0xFF; // 时间戳高位
    conn->rtp_header[5] = (frame_pts >> 16) & 0xFF; // 时间戳中位
    conn->rtp_header[6] = (frame_pts >> 8) & 0xFF;  // 时间戳低位
    conn->rtp_header[7] = (frame_pts) & 0xFF;       // 时间戳低位
}

/**
 * @brief 发送视频流处理
 *
 * 此函数处理视频帧的发送，构建RTP包并通过UDP发送。
 *
 * @param conn 指向Udp_Connection_S结构的指针，用于存储连接信息
 * @param frame_data 指向要发送的帧数据
 * @param frame_size 帧数据的大小
 * @param frame_pts 当前帧的时间戳
 * @return 返回0表示成功，-1表示失败
 */
int rtp_udp_push(Udp_Connection_S *conn, uint8_t *frame_data, size_t frame_size, uint32_t frame_pts)
{
    uint8_t udp_packet[UDP_PACKET_SIZE]; // 定义一个缓冲区用作UDP包

    // 先处理剩余未发送的数据
    if (frame_size_end > 0)
    {
        memcpy(udp_packet, frame_data_end, frame_size_end); // 复制之前未发送的数据到UDP包

        build_rtp_packet(conn, frame_pts); // 构建RTP包头

        memcpy(udp_packet + frame_size_end, conn->rtp_header, RTP_HEADER_SIZE); // 复制RTP头

        memcpy(udp_packet + (frame_size_end + RTP_HEADER_SIZE), frame_data, (UDP_PACKET_SIZE - (frame_size_end + RTP_HEADER_SIZE))); // 复制帧数据

        ssize_t sent = sendto(conn->sock, udp_packet, UDP_PACKET_SIZE, 0, (struct sockaddr *)&conn->addr, sizeof(conn->addr));
        if (sent < 0)
        {
            fprintf(stderr, "Failed to sendto UDP packet end+star\n");
            return -1; // 发送失败，退出
        }

        conn->seq_num++; // 增加序列号

        memset(udp_packet, 0, UDP_PACKET_SIZE); // 清空UDP包

        // 更新帧数据和帧大小，为下一次发送准备
        frame_data = frame_data + (UDP_PACKET_SIZE - (frame_size_end + RTP_HEADER_SIZE));
        frame_size = frame_size - (UDP_PACKET_SIZE - (frame_size_end + RTP_HEADER_SIZE));

        memset(frame_data_end, 0, UDP_PACKET_SIZE); // 清空未发送的数据缓冲区
        frame_size_end = 0; // 重置未发送数据大小
    }

    // 计算可以发送的完整RTP包的数量
    uint8_t frame_pack_number = frame_size / RTP_PACKET_SIZE;
    frame_size_end = frame_size % RTP_PACKET_SIZE; // 更新未发送的剩余帧数据大小

    while (frame_pack_number > 0) // 逐个发送完整的RTP包
    {
        build_rtp_packet(conn, frame_pts); // 构建RTP包头

        memcpy(udp_packet, conn->rtp_header, RTP_HEADER_SIZE); // 复制RTP头

        memcpy(udp_packet + RTP_HEADER_SIZE, frame_data, RTP_PACKET_SIZE); // 复制帧数据

        frame_data += RTP_PACKET_SIZE; // 移动到下一个数据包的起始位置

        ssize_t sent = sendto(conn->sock, udp_packet, UDP_PACKET_SIZE, 0, (struct sockaddr *)&conn->addr, sizeof(conn->addr)); // 发送UDP包
        if (sent < 0)
        {
            fprintf(stderr, "Failed to sendto UDP packet: %d\n", frame_pack_number);
            return -1; // 发送失败，退出
        }

        conn->seq_num++; // 增加序列号
        frame_pack_number--; // 减少包数量

        memset(udp_packet, 0, UDP_PACKET_SIZE); // 清空UDP包
    }

    // 处理剩余不足一个完整RTP包的数据
    if (frame_size_end > 0)
    {
        build_rtp_packet(conn, frame_pts); // 构建RTP包头
        // 将RTP头和剩余数据放入未发送数据缓冲区
        memcpy(frame_data_end, conn->rtp_header, RTP_HEADER_SIZE); // 复制RTP头
        memcpy(frame_data_end + RTP_HEADER_SIZE, frame_data, frame_size_end); // 复制剩余帧数据
        frame_size_end = frame_size_end + RTP_HEADER_SIZE; // 更新未发送数据大小
    }

    // 如果UDP包的大小减去剩余部分长度得到的可用长度小于下一个RTP包头+1，就不再和下一个包进行合并，而是直接发送它
    if ((frame_size_end > 0) && ((UDP_PACKET_SIZE - frame_size_end) < (RTP_HEADER_SIZE + 1)))
    {
        ssize_t sent = sendto(conn->sock, frame_data_end, frame_size_end, 0, (struct sockaddr *)&conn->addr, sizeof(conn->addr)); // 发送未发送数据
        if (sent < 0)
        {
            fprintf(stderr, "Failed to sendto UDP packet end\n");
            return -1; // 发送失败，退出
        }

        conn->seq_num++; // 增加序列号

        memset(frame_data_end, 0, UDP_PACKET_SIZE); // 清空未发送的数据缓冲区
        frame_size_end = 0; // 重置未发送数据大小
    }

    return 0; // 线程结束
}

/**
 * @brief 关闭UDP连接
 *
 * 此函数关闭给定的UDP连接并释放相关资源。
 *
 * @param conn 指向Udp_Connection_S结构的指针，用于存储连接信息
 * @return 返回0表示成功
 */
int rtp_udp_push_deinit(Udp_Connection_S *conn)
{
    if (conn->sock >= 0) // 检查socket是否有效
    {
        close(conn->sock); // 关闭socket
    }

    // 在释放之前验证
    if (frame_data_end) // 如果分配了未发送数据的缓冲区
    {
        free(frame_data_end); // 释放分配的内存
        frame_data_end = NULL; // 释放后设为NULL，防止悬空指针
    }

    return 0;
}

/**
 * @brief 主初始化函数
 *
 * 此函数初始化UDP连接和RTP，分配必要的内存。
 *
 * @param conn 指向Udp_Connection_S结构的指针，用于存储连接信息
 * @return 返回0表示成功，EXIT_FAILURE表示失败
 */
int rtp_udp_push_init(Udp_Connection_S *conn)
{
    // 初始化UDP连接
    if (0 != init_udp_connection(conn))
    {
        fprintf(stderr, "Failed to initialize UDP connection\n");
        return EXIT_FAILURE; // 程序初始化失败
    }

    // 初始化RTP
    if (0 != init_rtp_header(conn))
    {
        fprintf(stderr, "Failed to initialize RTP\n");
        return EXIT_FAILURE; // 程序初始化失败
    }

    // 分配内存用于未发送的帧数据
    frame_data_end = (uint8_t *)malloc(UDP_PACKET_SIZE * sizeof(uint8_t));
    if (frame_data_end == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for frame_data_end\n");
        return EXIT_FAILURE; // 程序初始化失败
    }

    memset(frame_data_end, 0, UDP_PACKET_SIZE);

    return 0; // 成功初始化
}