/* GStreamer
 * Copyright (C) 2013 Collabora Ltd.
 *   @author Torrie Fischer <torrie.fischer@collabora.co.uk>
 *
 * 这个程序实现了一个简单的 RTP 服务器，专注于视频流的推送
 * 并去除了音频部分、RTCP 支持和重传功能。
 */

#include <gst/gst.h>
#include <gst/rtp/rtp.h>

// 定义会话数据结构，包含 sessionNum 和输入元素信息
typedef struct _SessionData {
  int ref;                // 引用计数
  guint sessionNum;      // 会话编号
  GstElement *input;     // 对应的输入元素
} SessionData;

// 引用会话数据
static SessionData *session_ref(SessionData *data) {
  g_atomic_int_inc(&data->ref);
  return data;
}

// 取消引用会话数据，并在必要时释放它
static void session_unref(gpointer data) {
  SessionData *session = (SessionData *)data;
  if (g_atomic_int_dec_and_test(&session->ref)) {
    g_free(session);
  }
}

// 创建新的会话数据，初始化引用计数和会话编号
static SessionData *session_new(guint sessionNum) {
  SessionData *ret = g_new0(SessionData, 1);
  ret->sessionNum = sessionNum;
  return session_ref(ret);
}

// 回调函数，用于处理管道状态改变消息
static void cb_state(GstBus *bus, GstMessage *message, gpointer data) {
  GstObject *pipe = GST_OBJECT(data);
  GstState old, new, pending;
  gst_message_parse_state_changed(message, &old, &new, &pending);
  if (message->src == pipe) {
    g_print("Pipeline %s changed state from %s to %s\n",
            GST_OBJECT_NAME(message->src),
            gst_element_state_get_name(old), gst_element_state_get_name(new));
  }
}

// 创建视频会话，配置视频源、编码器和负载
static SessionData *make_video_session(guint sessionNum) {
  GstBin *videoBin = GST_BIN(gst_bin_new(NULL));
  GstElement *videoSrc = gst_element_factory_make("videotestsrc", NULL); // 视频测试源
  GstElement *encoder = gst_element_factory_make("theoraenc", NULL); // Theora 编码器
  GstElement *payloader = gst_element_factory_make("rtptheorapay", NULL); // RTP 负载器
  GstCaps *videoCaps;
  SessionData *session;

  // 设置视频源属性
  g_object_set(videoSrc, "is-live", TRUE, "horizontal-speed", 1, NULL);

  // 添加元素到视频 bin
  gst_bin_add_many(videoBin, videoSrc, encoder, payloader, NULL);
  
  // 创建视频格式的 caps
  videoCaps = gst_caps_new_simple("video/x-raw",
                                   "width", G_TYPE_INT, 352,
                                   "height", G_TYPE_INT, 288,
                                   "framerate", GST_TYPE_FRACTION, 15, 1,
                                   NULL);
  
  // 链接视频元素
  gst_element_link_filtered(videoSrc, encoder, videoCaps);
  gst_element_link(encoder, payloader);

  // 将视频 bin 设置为会话输入
  session = session_new(sessionNum);
  session->input = GST_ELEMENT(videoBin);

  return session;
}

// 添加视频流到管道，设置相应的 UDP 发送器
static void add_stream(GstPipeline *pipe, GstElement *rtpBin, SessionData *session) {
  GstElement *rtpSink = gst_element_factory_make("udpsink", NULL); // 创建 UDP 发送器
  int basePort;

  basePort = 5000 + (session->sessionNum * 2); // 计算基端口，确保不同会话使用不同端口

  gst_bin_add(GST_BIN(pipe), rtpSink); // 将 UDP 发送器添加到管道
  
  // 设置 UDP 发送器的属性
  g_object_set(rtpSink, "port", basePort, "host", "127.0.0.1", NULL);

  // 链接会话的输入到 rtpBin 的相应 pads
  gchar *padName = g_strdup_printf("send_rtp_sink_%u", session->sessionNum);
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
int main(int argc, char **argv) {
  GstPipeline *pipe;      // GStreamer 管道
  GstBus *bus;            // GStreamer 总线
  SessionData *videoSession; // 视频会话数据
  GstElement *rtpBin;     // RTP 处理元素
  GMainLoop *loop;        // GMainLoop 用于主循环

  gst_init(&argc, &argv); // 初始化 GStreamer

  loop = g_main_loop_new(NULL, FALSE); // 创建主循环

  pipe = GST_PIPELINE(gst_pipeline_new(NULL)); // 创建新的管道
  bus = gst_element_get_bus(GST_ELEMENT(pipe)); // 获取总线
  g_signal_connect(bus, "message::state-changed", G_CALLBACK(cb_state), pipe); // 连接状态改变信号
  gst_bus_add_signal_watch(bus); // 添加信号监视
  gst_object_unref(bus); // 取消引用总线

  rtpBin = gst_element_factory_make("rtpbin", NULL); // 创建 rtpbin 元素
  gst_bin_add(GST_BIN(pipe), rtpBin); // 将 rtpbin 添加到管道

  videoSession = make_video_session(0); // 创建视频会话
  add_stream(pipe, rtpBin, videoSession); // 添加视频流

  g_print("starting server pipeline\n");
  gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING); // 设置管道为播放状态

  g_main_loop_run(loop); // 运行主循环

  g_print("stopping server pipeline\n");
  gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_NULL); // 设置管道为 NULL 状态以停止播放

  gst_object_unref(pipe); // 取消引用管道
  g_main_loop_unref(loop); // 取消引用主循环

  return 0; // 程序结束
}