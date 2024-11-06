#include <stdio.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <glib.h>

static gboolean bus_call(GstBus *bus, GstMessage *msg, GMainLoop *loop) {
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: // 如果收到 EOS 消息，表示流结束
            g_print("End-Of-Stream reached.\n");
            g_main_loop_quit(loop); // 退出主循环
            break;
        case GST_MESSAGE_ERROR: { // 如果收到错误消息
            gchar *debug;
            GErrorError *error;

            gst_message_parse_error(msg, &error, &debug);
            g_printerr("Error received from element %s: %s\n", 
                        GST_OBJECT_NAME(msg->src), error->message);
            g_printerr("Debugging information: %s\n", debug ? debug : "none");
            g_clear_error(&error); // 清除错误
            g_free(debug); // 释放调试信息
            g_main_loop_quit(loop); // 退出主循环
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop; // 主循环
    GstElement *pipeline, *udpsrc, *rtpbin, *depayloader, *decoder, *videosink; // GStreamer 元素
    GstBus *bus; // 总线

    // 初始化 GStreamer
    gst_init(&argc, &argv);

    // 创建 GMainLoop
    loop = g_main_loop_new(NULL, FALSE);

    // 创建 GStreamer 元素
    udpsrc = gst_element_factory_make("udpsrc", "source"); // 创建 UDP 源
    rtpbin = gst_element_factory_make("rtpbin", "rtpbin"); // 创建 RTP 处理器
    depayloader = gst_element_factory_make("rtph265depay", "depayloader"); // H.265 RTP 负载器
    decoder = gst_element_factory_make("avdec_h265", "decoder"); // H.265 解码器
    videosink = gst_element_factory_make("autovideosink", "videosink"); // 视频输出窗口

    // 检查是否成功创建元素
    if (!udpsrc || !rtpbin || !depayloader || !decoder || !videosink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // 设置 udpsrc 参数
    g_object_set(udpsrc, "port", 5002, NULL); // 设定接收的端口
    g_object_set(udpsrc, "caps", gst_caps_new_simple("application/x-rtp",
                                                       "media", G_TYPE_STRING, "video",
                                                       "clock-rate", G_TYPE_INT, 90000,
                                                       "encoding-name", G_TYPE_STRING, "H265",
                                                       NULL), NULL); // 设置 RTP 的各种参数

    // 创建管道
    pipeline = gst_pipeline_new("receive-pipeline"); // 创建新的管道
    gst_bin_add_many(GST_BIN(pipeline), udpsrc, rtpbin, depayloader, decoder, videosink, NULL); // 将元素添加到管道中
    
    // 连接元素
    gst_element_link(udpsrc, rtpbin); // 连接 UDP 源到 RTP 处理器
    g_signal_connect(rtpbin, "pad-added", G_CALLBACK(gst_element_link), depayloader); // 连接动态 pad
    gst_element_link(depayloader, decoder); // 连接负载器到解码器
    gst_element_link(decoder, videosink); // 将解码器连接到视频输出

    // 获取总线并设置消息回调
    bus = gst_element_get_bus(pipeline); // 获取总线
    gst_bus_add_watch(bus, (GstBusFunc)bus_call, loop); // 设置消息回调
    gst_object_unref(bus); // 释放总线对象

    // 启动管道
    gst_element_set_state(pipeline, GST_STATE_PLAYING); // 设置管道状态为播放

    // 开始主循环
    g_print("Receiving RTP stream... Press Ctrl+C to stop.\n");
    g_main_loop_run(loop); // 运行主循环

    // 释放资源
    gst_element_set_state(pipeline, GST_STATE_NULL); // 设置管道状态为 NULL
    gst_object_unref(pipeline); // 释放管道资源
    g_main_loop_unref(loop); // 释放主循环资源

    return 0;
}