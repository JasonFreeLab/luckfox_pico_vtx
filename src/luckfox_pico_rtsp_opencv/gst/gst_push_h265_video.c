#include <stdio.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 640                     // 视频宽度
#define HEIGHT 480                    // 视频高度
#define FPS 30                        // 帧率
#define BUFFER_SIZE (WIDTH * HEIGHT)  // 假设为YUV420格式（该值可以根据实际情况调整）

// 模拟生成 H.265 编码的数据（实际使用时请替换为真实的数据）
static void fill_h265_frame(uint8_t *buffer) {
    memset(buffer, 0, BUFFER_SIZE); // 使用零填充缓冲区
}

// 推送数据的函数
static gboolean push_data(GstElement *appsrc) {
    GstBuffer *buffer;
    guint size = BUFFER_SIZE;          // 设置帧大小
    uint8_t *data = malloc(size);      // 分配内存用于存放视频帧数据

    if (!data) {
        g_printerr("无法分配内存用于视频帧。\n");
        return FALSE;
    }

    // 填充 H.265 编码帧数据
    fill_h265_frame(data);

    // 创建 GstBuffer
    buffer = gst_buffer_new_and_alloc(size);
    gst_buffer_fill(buffer, 0, data, size); // 向缓冲区填充数据
    
    // 设置时间戳和持续时间
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, FPS); // 每帧的持续时间
    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(0, GST_SECOND, FPS); // 时间戳

    // 推送数据
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);

    if (ret != GST_FLOW_OK) {
        g_printerr("推送缓冲区时出错: %d\n", ret);
    }

    free(data); // 释放内存

    return ret == GST_FLOW_OK;
}

// 处理总线消息的回调函数
static gboolean bus_call(GstBus *bus, GstMessage *msg, GMainLoop *loop) {
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: // 如果收到 EOS 消息
            g_print("到达流结束。\n");
            g_main_loop_quit(loop); // 退出主循环
            break;
        case GST_MESSAGE_ERROR: { // 如果收到错误消息
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("来自元素 %s 的错误: %s\n", GST_OBJECT_NAME(msg->src), error->message);
            g_printerr("调试信息: %s\n", debug ? debug : "无");
            g_clear_error(&error);
            g_free(debug);
            g_main_loop_quit(loop); // 退出主循环
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstElement *pipeline, *appsrc, *payloader, *rtpbin, *sink;
    GstBus *bus;

    // 初始化 GStreamer
    gst_init(&argc, &argv);
    
    // 创建 GMainLoop
    loop = g_main_loop_new(NULL, FALSE);

    // 创建 GStreamer 元素
    appsrc = gst_element_factory_make("appsrc", "source");
    payloader = gst_element_factory_make("rtph265pay", "payloader"); // H.265 RTP 负载器
    rtpbin = gst_element_factory_make("rtpbin", "rtpbin"); // RTP 处理器
    sink = gst_element_factory_make("udpsink", "sink"); // UDP 接收器

    // 检查是否成功创建元素
    if (!appsrc || !payloader || !rtpbin || !sink) {
        g_printerr("无法创建所有元素。\n");
        return -1;
    }

    // 设置 UDP sink 参数
    g_object_set(sink, "host", "127.0.0.1", NULL);  // 设置目标主机
    g_object_set(sink, "port", 5002, NULL);          // 设置目标端口
    
    // 设置 appsrc 的 caps
    g_object_set(appsrc, "caps", gst_caps_new_simple("video/x-h265",
                                                       "width", G_TYPE_INT, WIDTH,
                                                       "height", G_TYPE_INT, HEIGHT,
                                                       "framerate", GST_TYPE_FRACTION, FPS, 1,
                                                       NULL), NULL);

    // 创建管道
    pipeline = gst_pipeline_new("video-pipeline");
    gst_bin_add_many(GST_BIN(pipeline), appsrc, rtpbin, payloader, sink, NULL);
    gst_element_link(appsrc, payloader); // 将应用源链接到 RTP 负载器
    gst_element_link(payloader, rtpbin);  // 将负载器链接到 RTP 处理器
    gst_element_link(rtpbin, sink);       // 将 RTP 处理器链接到 UDP sink

    // 获取总线并设置消息回调
    bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)bus_call, loop); // 设置总线回调
    gst_object_unref(bus);

    // 启动管道
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // 持续推送数据
    for (int i = 0; i < 100; i++) { // 推送100帧
        if (!push_data(appsrc)) {
            break; // 推送失败则退出循环
        }
        g_usleep(1000000 / FPS); // 控制帧率
    }

    // 发送 EOS
    gst_app_src_end_of_stream(GST_APP_SRC(appsrc));

    // 开始主循环
    g_print("程序正在运行... 按 Ctrl+C 停止。\n");
    g_main_loop_run(loop);

    // 释放资源
    gst_element_set_state(pipeline, GST_STATE_NULL); // 设置管道状态为 NULL
    gst_object_unref(pipeline); // 释放管道资源
    g_main_loop_unref(loop); // 释放主循环资源

    return 0;
}