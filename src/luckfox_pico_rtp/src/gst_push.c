#include <stdio.h>

#include "gst_push.h"

GstElement *pipeline, *appsrc, *parser, *rtp_payloader, *udpsink;
guint64 fps_time = 0;
GstBuffer *buffer;
GstFlowReturn ret;

// 获取视频帧并将其推送到管道
void gst_push_data(FrameData_S *frame)
{
    // 创建GStreamer的缓冲区
    buffer = gst_buffer_new_allocate(NULL, frame->size, NULL);

    // 填充视频帧数据
    gst_buffer_fill(buffer, 0, frame->buffer, frame->size);

    // 设置PTS
    GST_BUFFER_PTS(buffer) = frame->pts;

    // 设置每帧的持续时间
    GST_BUFFER_DURATION(buffer) = fps_time;

    // 推送缓冲区到appsrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    // 释放内存
    gst_buffer_unref(buffer);
}

int gst_push_init(GstPushInitParameter_S *gst_push_init_parameter)
{
    // 初始化GStreamer
    gst_init(NULL, NULL);

    // 创建GStreamer元素
    appsrc = gst_element_factory_make("appsrc", "source");
    parser = gst_element_factory_make(gst_push_init_parameter->encodec_type ? "h265parse" : "h264parse", "parser");
    rtp_payloader = gst_element_factory_make(gst_push_init_parameter->encodec_type ? "rtph265pay" : "rtph264pay", "rtp_payloader");
    udpsink = gst_element_factory_make("udpsink", "udp_sink");

    // 创建管道
    pipeline = gst_pipeline_new("video-pipeline");

    // 检查元素是否创建成功，失败则打印错误信息并退出
    if (!pipeline)
    {
        g_printerr("pipeline element could not be created. Exiting.\n");
        return -1;
    }
    if (!appsrc)
    {
        g_printerr("appsrc element could not be created. Exiting.\n");
        return -1;
    }
    if (!parser)
    {
        g_printerr("parser element could not be created. Exiting.\n");
        return -1;
    }
    if (!rtp_payloader)
    {
        g_printerr("rtp_payloader element could not be created. Exiting.\n");
        return -1;
    }
    if (!udpsink)
    {
        g_printerr("udpsink element could not be created. Exiting.\n");
        return -1;
    }

    // 设置udpsink的属性（目标主机和端口）
    g_object_set(udpsink, "host", gst_push_init_parameter->host_ip, NULL);
    g_object_set(udpsink, "port", gst_push_init_parameter->host_port, NULL);

    // 将元素添加到管道
    gst_bin_add_many(GST_BIN(pipeline), appsrc, parser, rtp_payloader, udpsink, NULL);

    // 链接管道中的元素，如果失败则打印错误信息并退出
    if (!gst_element_link_many(appsrc, parser, rtp_payloader, udpsink, NULL))
    {
        g_printerr("Elements could not be linked. Exiting.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // 设置appsrc的属性，以支持流式数据
    g_object_set(appsrc, "format", GST_FORMAT_TIME, NULL);
    g_object_set(appsrc, "is-live", TRUE, NULL);
    g_object_set(appsrc, "min-latency", 0, NULL);
    g_object_set(appsrc, "max-latency", 0, NULL);

    // 启动管道，切换到播放状态
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    fps_time = gst_util_uint64_scale(GST_SECOND, 1, gst_push_init_parameter->fps);

    return 0;
}

int gst_push_deinit(void)
{
    GstBus *bus;
    GstMessage *msg;

    // 发送EOS信号，指示流结束
    g_signal_emit_by_name(appsrc, "end-of-stream", NULL);

    // 处理消息
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS | GST_MESSAGE_ERROR);

    if (msg != NULL)
    {
        GError *err = NULL;
        gchar *debug_info = NULL;

        switch (GST_MESSAGE_TYPE(msg))
        {
        case GST_MESSAGE_ERROR: // 错误处理
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
            g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error(&err);
            g_free(debug_info);
            break;
        case GST_MESSAGE_EOS: // 流结束处理
            g_print("End-Of-Stream reached.\n");
            break;
        default: // 处理意外消息
            g_printerr("Unexpected message received.\n");
            break;
        }

        gst_message_unref(msg); // 释放消息对象
    }

    // 清理管道
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}