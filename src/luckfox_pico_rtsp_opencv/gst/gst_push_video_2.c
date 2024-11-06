#include <stdio.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

// 定义会话数据结构，包含 sessionNum 和输入元素信息
typedef struct _SessionData {
    int ref;                // 引用计数
    guint sessionNum;      // 会话编号
    GstElement *input;     // 对应的输入元素
} SessionData;

// 引用会话数据
static SessionData* session_ref(SessionData* data) {
    g_atomic_int_inc(&data->ref);
    return data;
}

// 取消引用会话数据，并在必要时释放它
static void session_unref(gpointer data) {
    SessionData* session = (SessionData*)data;
    if (g_atomic_int_dec_and_test(&session->ref)) {
        g_free(session);
    }
}

// 创建新的会话数据，初始化引用计数和会话编号
static SessionData* session_new(guint sessionNum) {
    SessionData* ret = g_new0(SessionData, 1);
    ret->sessionNum = sessionNum;
    return session_ref(ret);
}

// 填充 H.265 编码帧数据的函数（模拟）
static void fill_h265_frame(uint8_t* buffer, int frame_size) {
    memset(buffer, 0, frame_size); // 使用零填充缓冲区
}

// 推送数据的函数
static gboolean push_data(GstElement* appsrc) {
    GstBuffer* buffer;
    guint size = 640 * 480; // 假设为 YUV420 格式
    uint8_t* data = malloc(size); // 分配内存用于存放视频帧数据

    if (!data) {
        g_printerr("无法分配内存用于视频帧。\n");
        return FALSE;
    }

    // 填充 H.265 编码帧数据
    fill_h265_frame(data, size);

    // 创建 GstBuffer
    buffer = gst_buffer_new_and_alloc(size);
    gst_buffer_fill(buffer, 0, data, size); // 向缓冲区填充数据

    // 设置时间戳和持续时间
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30); // 每帧的持续时间
    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(0, GST_SECOND, 30); // 时间戳

    // 推送数据
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);

    free(data); // 释放内存
    return ret == GST_FLOW_OK;
}

// 创建视频会话，配置视频源、编码器和负载
static SessionData* make_video_session(guint sessionNum) {
    GstElement* appsrc = gst_element_factory_make("appsrc", NULL);
    GstElement* encoder = gst_element_factory_make("rtph265pay", NULL); // H.265 RTP 负载器
    GstElement* rtpBin = gst_element_factory_make("rtpbin", NULL); // RTP 处理器
    GstBin* videoBin = GST_BIN(gst_bin_new(NULL));
    GstCaps* videoCaps;

    // 设置 appsrc 的 caps
    g_object_set(appsrc, "caps", gst_caps_new_simple("video/x-h265",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL), NULL);

    // 添加元素到视频 bin
    gst_bin_add_many(videoBin, appsrc, encoder, NULL);
    
    // 将视频 bin 设置为会话输入
    SessionData* session = session_new(sessionNum);
    session->input = GST_ELEMENT(videoBin);

    // 链接元素
    gst_element_link(appsrc, encoder);
    // 需要稍后链接 rtpBin

    return session;
}

// 添加视频流到管道，设置相应的 UDP 发送器
static void add_stream(GstPipeline* pipe, GstElement* rtpBin, SessionData* session) {
    GstElement* rtpSink = gst_element_factory_make("udpsink", NULL); // 创建 UDP 发送器
    int basePort = 5000 + (session->sessionNum * 2); // 计算基端口

    gst_bin_add(GST_BIN(pipe), rtpSink); // 将 UDP 发送器添加到管道
    
    // 设置 UDP 发送器的属性
    g_object_set(rtpSink, "port", basePort, "host", "127.0.0.1", NULL);

    // 链接会话的输入到 rtpBin 的相应 pads
    gchar* padName = g_strdup_printf("send_rtp_sink_%u", session->sessionNum);
    gst_element_link_pads(session->input, "src", rtpBin, padName);
    g_free(padName);

    // 链接 rtpBin 到 UDP 发送器
    padName = g_strdup_printf("send_rtp_src_%u", session->sessionNum);
    gst_element_link_pads(rtpBin, padName, rtpSink, "sink");
    g_free(padName);

    // 打印新视频流的信息
    g_print("New RTP stream on %i\n", basePort);

    // 取消引用会话数据
    session_unref(session);
}

// 主函数，程序入口
int main(int argc, char** argv) {
    GstPipeline* pipe;      // GStreamer 管道
    GstBus* bus;            // GStreamer 总线
    SessionData* videoSession; // 视频会话数据
    GstElement* rtpBin;     // RTP 处理元素
    GMainLoop* loop;        // GMainLoop 用于主循环

    // 初始化 GStreamer
    gst_init(&argc, &argv);

    loop = g_main_loop_new(NULL, FALSE); // 创建主循环
    pipe = GST_PIPELINE(gst_pipeline_new(NULL)); // 创建新的管道
    
    bus = gst_element_get_bus(GST_ELEMENT(pipe));
    gst_object_unref(bus); // 取消引用总总线

    rtpBin = gst_element_factory_make("rtpbin", NULL); // 创建 rtpbin 元素
    gst_bin_add(GST_BIN(pipe), rtpBin); // 将 rtpbin 添加到管道

    videoSession = make_video_session(0); // 创建视频会话
    add_stream(pipe, rtpBin, videoSession); // 添加视频流

    g_print("starting server pipeline\n");
    gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING); // 设置管道为播放状态

    // 持续推送数据
    for (int i = 0; i < 100; i++) { // 推送100帧
        if (!push_data(GST_ELEMENT(videoSession->input))) {
            break; // 推送失败则退出循环
        }
        g_usleep(1000000 / 30); // 控制帧率
    }

    // 发送 EOS
    gst_app_src_end_of_stream(GST_APP_SRC(videoSession->input));

    g_main_loop_run(loop); // 运行主循环

    g_print("stopping server pipeline\n");
    gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_NULL); // 设置管道为 NULL 状态以停止播放

    gst_object_unref(pipe); // 取消引用管道
    g_main_loop_unref(loop); // 取消引用主循环

    return 0; // 程序结束
}