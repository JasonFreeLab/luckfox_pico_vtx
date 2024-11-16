#include <gst/gst.h>          // 包含GStreamer库的头文件

#define FRAME_SIZE 1024      // 假设的每一帧的大小

// 全局变量，指向GStreamer管道和appsrc元素
static GstElement *pipeline;
static GstElement *appsrc;
static gboolean is_streaming = TRUE; // 标志，表示是否继续流媒体推送

// 将数据缓冲区推送到GStreamer管道的函数
static void send_buffer_to_appsrc(GstBuffer *buffer) {
    // 将buffer推送到管道
    g_signal_emit_by_name(appsrc, "push-buffer", buffer);
}

// 处理请求数据的回调
static GstFlowReturn on_need_data(GstElement *sink, gpointer user_data) {
    // 如果不再流媒体，则返回结束信号
    if (!is_streaming) {
        return GST_FLOW_EOS; 
    }

    unsigned char *frame_data = (unsigned char *)user_data; // 获取帧数据的的指针
    GstBuffer *buffer = gst_buffer_new_and_alloc(FRAME_SIZE); // 创建新的GstBuffer
    gst_buffer_fill(buffer, 0, frame_data, FRAME_SIZE); // 填充缓冲区数据

    // 推送数据到GStreamer
    send_buffer_to_appsrc(buffer);
    gst_buffer_unref(buffer); // 解引用缓冲区以释放内存

    return GST_FLOW_OK; // 表示成功
}

// 创建并初始化GStreamer管道
static void create_gstreamer_pipeline() {
    gst_init(NULL, NULL); // 初始化GStreamer

    // 创建appsrc（数据源）、rtph264pay（RTP打包器）和udpsink（UDP接收器）元素
    appsrc = gst_element_factory_make("appsrc", "source");
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "payloader");
    GstElement *udpsink = gst_element_factory_make("udpsink", "sink");

    // 检查元素是否成功创建
    if (!appsrc || !rtph264pay || !udpsink) {
        g_printerr("Not all elements could be created.\n");
        exit(EXIT_FAILURE);
    }

    // 创建GStreamer管道
    pipeline = gst_pipeline_new("video-pipeline");

    // 设置UDP目标，发送数据到127.0.0.1的5000端口
    g_object_set(udpsink, "host", "127.0.0.1", NULL);
    g_object_set(udpsink, "port", 5000, NULL);

    // 将元素添加到管道
    gst_bin_add_many(GST_BIN(pipeline), appsrc, rtph264pay, udpsink, NULL);
    
    // 链接元素
    if (gst_element_link(appsrc, rtph264pay) != TRUE || 
        gst_element_link(rtph264pay, udpsink) != TRUE) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        exit(EXIT_FAILURE);
    }
    
    // 设置appsrc的属性，指定数据格式为H.264
    g_object_set(appsrc, "caps", gst_caps_from_string("video/x-h264"), NULL);
    g_object_set(appsrc, "format", GST_FORMAT_TIME, NULL);
    
    // 设置appsrc的callback，当需要更多数据时会调用此回调
    g_signal_connect(appsrc, "need-data", G_CALLBACK(on_need_data), NULL);
}

// 启动GStreamer管道
static void play_pipeline() {
    gst_element_set_state(pipeline, GST_STATE_PLAYING); // 将管道状态设置为播放
}

// 停止并清理管道
static void stop_and_cleanup_pipeline() {
    is_streaming = FALSE; // 停止流的标志
    gst_element_set_state(pipeline, GST_STATE_NULL); // 将管道状态设置为NULL（停止）
    gst_object_unref(pipeline); // 释放管道资源
}

int main_init(void) {
    unsigned char video_frame_data[FRAME_SIZE]; // 假设这里存放视频帧数据

    // 创建并初始化GStreamer管道
    create_gstreamer_pipeline();

    // 启动管道
    play_pipeline();

    // 在这里循环创建并推送帧数据
    for (int i = 0; i < 10; i++) {  // 推送10帧示例
        memset(video_frame_data, i, FRAME_SIZE);  // 使用示例数据填充帧
        send_buffer_to_appsrc(gst_buffer_new_and_alloc(FRAME_SIZE)); // 推送数据
        g_usleep(100000);  // 等待100ms，模拟帧率
    }

    // 停止并清理管道
    stop_and_cleanup_pipeline();

    return 0; // 返回0表示程序正常退出
}