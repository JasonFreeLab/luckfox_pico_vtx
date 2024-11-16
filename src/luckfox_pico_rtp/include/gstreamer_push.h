#ifndef GSTREAMER_PUSH_H
#define GSTREAMER_PUSH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <glib.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>


// 定义gst参数的结构体
typedef struct _Gst_Element {
    GstElement *appsrc;
    GstElement *encoder;
    GstElement *udpsink;
} Gst_Element;

// 定义视频参数的结构体
typedef struct _InputParameters {
    const char* g_host;     // 目标主机
    uint16_t g_port;        // 目标端口
    uint16_t width;         // 视频宽度
    uint16_t height;        // 视频高度
    uint8_t fps;            // 帧率
    const char* codec;      // 编码格式，如 "video/x-h265"
    const char* type;       // 编码器类型，例如 "rtph265pay"
    void* frameData;
    guint frameSize;
    guint64 pts;
    guint framesSent;       // 已发送的帧计数
    Gst_Element* gst_element;
} InputParameters;


// 函数声明
void gstreamer_push_init(InputParameters* params);  // 初始化函数
void gstreamer_push_deinit(InputParameters* params);    // 去初始化函数
gboolean gstreamer_push_frame_data(InputParameters* params);  // 发送视频帧函数

#endif // GSTREAMER_PUSH_H