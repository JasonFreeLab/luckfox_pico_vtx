#ifndef __GST_PUSH_H
#define __GST_PUSH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>


// 获取编码后的视频帧
typedef struct
{
    uint8_t *buffer; // 视频帧数据
    size_t size;     // 视频帧大小
    guint64 pts;     // PTS（显示时间戳）
} FrameData_S;

typedef enum
{
    EncondecType_E_H264 = 0,
    EncondecType_E_H265 = 1
} EncondecType_E;

typedef struct
{
    char *host_ip; // 目标主机IP
    uint16_t host_port; // 目标主机端口号
    EncondecType_E encodec_type;     // 视频帧编码类型
    guint64 fps;     // FPS
} GstPushInitParameter_S;

void gst_push_data(FrameData_S *frame);
int gst_push_init(GstPushInitParameter_S * gst_push_init_parameter);
int gst_push_deinit(void);

#endif //__GST_PUSH_H