#include <stdio.h>

#include "gst_push.h"

// GstElement *pipeline, *appsrc, *parser, *rtp_payloader, *udpsink, *queue; // 定义GStreamer元素的指针
GstElement *pipeline, *appsrc, *parser, *rtp_payloader, *udpsink; // 定义GStreamer元素的指针
guint64 fps_time = 0;                                             // 定义用于存储每帧持续时间的变量
GstBuffer *buffer;                                                // 定义GStreamer缓冲区的指针
GstFlowReturn ret;                                                // 定义用于存储GStreamer流处理返回值的变量

/**
 * @brief 获取视频帧并将其推送到管道
 *
 * @param frame 指向 FrameData_S 结构体的指针，包含视频帧数据
 *
 * @return int 返回0表示成功，返回-1表示失败
 *
 * 本函数创建一个GStreamer缓冲区，填充视频帧数据，设置时间戳并将缓冲区推送到appsrc元素。
 */
int gst_push_data(FrameData_S *frame)
{
    // 创建GStreamer的缓冲区，分配足够的内存以容纳帧数据
    buffer = gst_buffer_new_allocate(NULL, frame->size, NULL); // 根据帧数据大小分配缓冲区
    if (buffer == NULL)                                        // 检查缓冲区是否成功创建
    {
        g_printerr("Failed to create buffer.\n"); // 打印错误信息
        return -1;                                // 返回失败状态
    }

    // 填充视频帧数据到缓冲区
    gst_buffer_fill(buffer, 0, frame->buffer, frame->size); // 将帧数据拷贝到新创建的缓冲区中

    // 设置PTS（Presentation Timestamp）
    GST_BUFFER_PTS(buffer) = frame->pts; // 设置缓冲区的时间戳为帧数据的时间戳

    // 设置每帧的持续时间
    GST_BUFFER_DURATION(buffer) = fps_time; // 根据帧持续时间设置缓冲区的持续时间

    // 推送缓冲区到appsrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret); // 通过GStreamer信号将缓冲区推送到appsrc元素
    if (ret != GST_FLOW_OK)                                     // 检查推送是否成功
    {
        g_printerr("Error pushing buffer to appsrc: %d\n", ret); // 打印错误信息
        return -1;                                               // 返回失败状态
    }

    // 释放内存
    gst_buffer_unref(buffer); // 释放缓冲区占用的内存

    return 0; // 返回成功状态
}

/**
 * @brief 初始化GStreamer管道
 *
 * @param gst_push_init_parameter 指向 GstPushInitParameter_S 结构体的指针，包含初始化参数
 * @return int 返回0表示成功，返回-1表示失败
 *
 * 本函数初始化GStreamer，创建所需的GStreamer元素，链接它们并设置属性。还会根据传入的帧率计算每帧的持续时间。
 */
int gst_push_init(GstPushInitParameter_S *gst_push_init_parameter)
{
    // 初始化GStreamer
    gst_init(NULL, NULL); // 初始化GStreamer库，以便使用其功能

    // 创建GStreamer元素
    appsrc = gst_element_factory_make("appsrc", "source");                                                                          // 创建应用程序源元素appsrc
    parser = gst_element_factory_make(gst_push_init_parameter->encodec_type ? "h265parse" : "h264parse", "parser");                 // 根据编码类型选择解析器
    rtp_payloader = gst_element_factory_make(gst_push_init_parameter->encodec_type ? "rtph265pay" : "rtph264pay", "rtp_payloader"); // 根据编码类型选择RTP打包元素
    udpsink = gst_element_factory_make("udpsink", "udp_sink");                                                                      // 创建UDP接收器元素
    // queue = gst_element_factory_make("queue", "queue");                                                                             // 创建队列元素

    // 创建一个新的GStreamer管道
    pipeline = gst_pipeline_new("video-pipeline"); // 创建新的GStreamer管道

    // 检查所有元素是否成功创建，任何失败都打印相应的错误信息并退出
    // if (!pipeline || !appsrc || !parser || !rtp_payloader || !udpsink || !queue)
    if (!pipeline || !appsrc || !parser || !rtp_payloader || !udpsink)
    {
        g_printerr("Failed to create one or more GStreamer elements. Exiting.\n"); // 打印错误信息
        return -1;                                                                 // 返回失败状态
    }

    // 设置appsrc元素的属性，以支持实时数据流
    GstCaps *caps = gst_caps_new_simple(gst_push_init_parameter->encodec_type ? "video/x-h265" : "video/x-h264", 
                                        "stream-format", G_TYPE_STRING, "byte-stream", 
                                        NULL);
    g_object_set(appsrc, "caps", caps, NULL);
    gst_caps_unref(caps);
    g_object_set(appsrc, "format", GST_FORMAT_TIME, NULL); // 设置数据格式为时间格式
    g_object_set(appsrc, "is-live", TRUE, NULL);           // 指定appsrc是一个实时数据源
    g_object_set(appsrc, "min-latency", 0, NULL);          // 设置appsrc的最小延迟
    g_object_set(appsrc, "max-latency", 0, NULL);          // 设置appsrc的最大延迟

    // 设置udpsink元素的目标主机和端口
    g_object_set(udpsink, "host", gst_push_init_parameter->host_ip, NULL);   // 设置UDP目标主机IP
    g_object_set(udpsink, "port", gst_push_init_parameter->host_port, NULL); // 设置UDP目标端口
    g_object_set(udpsink, "sync", FALSE, NULL);                              // 设置为不使用同步，立即发送数据

    // 设置队列的属性
    // g_object_set(queue, "max-size-buffers", 1, NULL); // 设置队列最大缓存1个缓冲区
    // g_object_set(queue, "max-size-bytes", 0, NULL);   // 最大缓存字节数为无限制
    // g_object_set(queue, "max-size-time", 0, NULL);    // 最大缓存时间无限制

    // 将所有创建的元素添加到管道中
    // gst_bin_add_many(GST_BIN(pipeline), appsrc, parser, queue, rtp_payloader, udpsink, NULL); // 添加元素到管道
    gst_bin_add_many(GST_BIN(pipeline), appsrc, parser, rtp_payloader, udpsink, NULL); // 添加元素到管道

    // 链接管道中的所有元素，如果链接失败则打印错误并退出
    // if (!gst_element_link_many(appsrc, parser, queue, rtp_payloader, udpsink, NULL))
    if (!gst_element_link_many(appsrc, parser, rtp_payloader, udpsink, NULL))
    {
        g_printerr("Elements could not be linked. Exiting.\n"); // 打印错误信息
        // gst_bin_remove_many(GST_BIN(pipeline), appsrc, parser, queue, rtp_payloader, udpsink, NULL); // 移除已添加的元素
        gst_bin_remove_many(GST_BIN(pipeline), appsrc, parser, rtp_payloader, udpsink, NULL); // 移除已添加的元素
        gst_object_unref(pipeline);                                                           // 释放管道资源
        return -1;                                                                            // 返回失败状态
    }

    // 启动管道，切换到播放状态
    gst_element_set_state(pipeline, GST_STATE_PLAYING); // 将管道状态设置为播放

    // 计算帧持续时间
    if (gst_push_init_parameter->fps > 0)
    {                                                                                  // 如果帧率大于0
        fps_time = gst_util_uint64_scale(GST_SECOND, 1, gst_push_init_parameter->fps); // 根据帧率计算每帧的持续时间
    }
    else
    {
        g_printerr("FPS must be greater than 0.\n");         // 打印错误信息
        fps_time = gst_util_uint64_scale(GST_SECOND, 1, 30); // 默认帧率为30fps的持续时间
    }

    return 0; // 返回成功状态
}

/**
 * @brief 清理和释放GStreamer管道资源
 *
 * @return int 返回0表示成功
 *
 * 本函数发送EOS（End Of Stream）信号，处理消息，然后清理管道相关资源。
 */
int gst_push_deinit(void)
{
    GstBus *bus;     // 定义消息总线变量
    GstMessage *msg; // 定义消息变量

    // 发送EOS信号以指示流的结束
    g_signal_emit_by_name(appsrc, "end-of-stream", NULL); // 发送结束流信号

    // 获取管道的消息总线
    bus = gst_element_get_bus(pipeline); // 获取消息总线以便监听消息

    // 从总线中提取EOS或错误消息
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS | GST_MESSAGE_ERROR); // 从总线中提取消息

    // 检查是否有消息可处理
    if (msg != NULL) // 如果成功接收到消息
    {
        GError *err = NULL;       // 定义错误信息变量
        gchar *debug_info = NULL; // 定义调试信息变量

        switch (GST_MESSAGE_TYPE(msg)) // 根据消息类型处理相应的消息
        {
        case GST_MESSAGE_ERROR:                                                                          // 错误消息处理
            gst_message_parse_error(msg, &err, &debug_info);                                             // 解析错误消息
            g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message); // 打印元素及错误信息
            g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");                 // 打印调试信息
            g_clear_error(&err);                                                                         // 清理错误信息结构
            g_free(debug_info);                                                                          // 释放调试信息
            break;
        case GST_MESSAGE_EOS:                    // EOS消息处理
            g_print("End-Of-Stream reached.\n"); // 打印流结束信息
            break;
        default:                                          // 处理意外的消息类型
            g_printerr("Unexpected message received.\n"); // 打印意外消息信息
            break;
        }

        gst_message_unref(msg); // 释放消息对象
    }

    // 释放所有资源
    gst_object_unref(bus);                           // 释放总线资源
    gst_element_set_state(pipeline, GST_STATE_NULL); // 将管道状态设置为NULL，以释放资源
    gst_object_unref(pipeline);                      // 释放管道资源

    return 0; // 返回成功状态
}