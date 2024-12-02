#ifndef __GST_PUSH_H
#define __GST_PUSH_H

#include <stdint.h>             // 引入标准整数定义，以便使用uint8_t等类型
#include <gst/gst.h>            // 引入GStreamer核心库
#include <gst/app/gstappsink.h> // 引入GStreamer应用程序接收器
#include <gst/app/gstappsrc.h>  // 引入GStreamer应用程序源

// 定义一个结构体，用于获取编码后的视频帧数据
typedef struct
{
    uint8_t *buffer; // 指向视频帧数据的指针
    size_t size;     // 视频帧的大小
    uint64_t pts;    // PTS（显示时间戳），用于同步
} FrameData_S;

// 枚举类型，用于表示支持的视频编码格式
typedef enum
{
    EncondecType_E_H264 = 0, // H.264 编码类型
    EncondecType_E_H265 = 1  // H.265 编码类型
} EncondecType_E;

// 定义一个结构体，用于存储GStreamer初始化参数
typedef struct
{
    char *host_ip;               // 目标主机IP地址
    uint16_t host_port;          // 目标主机的端口号
    EncondecType_E encodec_type; // 视频帧的编码类型（H.264或H.265）
    uint64_t fps;                // 帧率（Frames Per Second）
} GstPushInitParameter_S;

/**
 * @brief 获取视频帧并将其推送到管道
 *
 * @param frame 指向 FrameData_S 结构体的指针，包含视频帧数据
 *
 * 本函数创建一个GStreamer缓冲区，填充视频帧数据，设置时间戳并将缓冲区推送到appsrc元素。
 */
int gst_push_data(FrameData_S *frame);

/**
 * @brief 初始化GStreamer管道
 *
 * @param gst_push_init_parameter 指向 GstPushInitParameter_S 结构体的指针，包含初始化参数
 * @return int 返回0表示成功，返回-1表示失败
 *
 * 本函数初始化GStreamer，创建所需的GStreamer元素，链接它们并设置属性。还会根据传入的帧率计算每帧的持续时间。
 */
int gst_push_init(GstPushInitParameter_S *gst_push_init_parameter);

/**
 * @brief 清理和释放GStreamer管道资源
 *
 * @return int 返回0表示成功
 *
 * 本函数发送EOS（End Of Stream）信号，处理消息，然后清理管道相关资源。
 */
int gst_push_deinit(void);

#endif //__GST_PUSH_H