#ifndef __GST_PUSH_H
#define __GST_PUSH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

#define HOST_IP "127.0.0.1" // 目标主机IP
#define HOST_PORT 5000      // 目标主机端口号

// 获取编码后的视频帧
typedef struct
{
    uint8_t *buffer; // 视频帧数据
    size_t size;     // 视频帧大小
    guint64 pts;     // PTS（显示时间戳）
} FrameData;

void gst_push_data(FrameData *frame);
int gst_push_init(void);
int gst_push_deinit(void);

#endif //__GST_PUSH_H