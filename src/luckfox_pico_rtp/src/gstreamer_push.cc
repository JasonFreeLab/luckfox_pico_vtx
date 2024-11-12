#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

// 定义视频参数的结构体
typedef struct _VideoParameters {
    int width;         // 视频宽度
    int height;        // 视频高度
    int fps;           // 帧率
    const char* codec; // 编码格式，如 "video/x-h265"
} VideoParameters;

// 定义编码器参数的结构体
typedef struct _EncoderParameters {
    const char* type;  // 编码器类型，例如 "rtph265pay"
    const char* codec; // 对应的编码格式，例如 "video/x-h265"
} EncoderParameters;

// 定义输入线程的结构体
typedef struct _InputThread {
    volatile int run; // 控制线程的运行状态
    GThread* thread;  // 线程句柄
} InputThread;

// 定义会话数据结构体
typedef struct _SessionData {
    int ref;                // 引用计数
    guint sessionNum;      // 会话编号
    GstElement *input;     // 输入元素，如 appsrc
    GstElement *encoder;   // 编码器元素
    GMutex mutex;          // 互斥锁，用于线程安全
    GCond cond;            // 条件变量，用于线程同步
    uint8_t *frameData;    // 存储视频帧数据的指针
    guint frameSize;       // 当前帧的大小
    guint64 pts;           // 当前帧的时间戳
    int frameReady;        // 标志，表示帧是否准备好
    int framesSent;        // 已发送的帧计数
} SessionData;

// 日志记录函数
void log_message(const char* message) {
    printf("[%s] %s\n", g_get_current_time(), message);
}

// 创建新的会话数据，并初始化
static SessionData* session_new(guint sessionNum) {
    SessionData* ret = g_new0(SessionData, 1);
    ret->sessionNum = sessionNum;   // 设置会话编号
    g_mutex_init(&ret->mutex);       // 初始化互斥锁
    g_cond_init(&ret->cond);         // 初始化条件变量
    return ret;                     // 返回新的会话数据
}

// 管理帧数据并推送到 GStreamer
static gboolean push_frame_data(SessionData* session, uint8_t* frameData, guint frameSize, guint64 pts) {
    g_mutex_lock(&session->mutex); // 锁定互斥锁

    // 释放旧的帧数据
    if (session->frameData) {
        free(session->frameData);
    }

    // 设置新的帧数据
    session->frameData = frameData;
    session->frameSize = frameSize;
    session->pts = pts;
    session->frameReady = 1; // 标记帧已准备好

    g_cond_signal(&session->cond); // 唤醒等待的线程
    g_mutex_unlock(&session->mutex); // 解锁互斥锁

    // 推送数据到 GStreamer 流水线
    GstBuffer* buffer = gst_buffer_new_and_alloc(session->frameSize);
    gst_buffer_fill(buffer, 0, session->frameData, session->frameSize); // 填充帧数据
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30); // 固定的帧率
    GST_BUFFER_PTS(buffer) = session->pts; // 设置PTS

    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(session->input), buffer);
    gst_buffer_unref(buffer); // 释放 GstBuffer

    if (ret != GST_FLOW_OK) {
        log_message("将缓冲区推送到 appsrc 时失败");
        return FALSE; // 推送失败
    }
    
    session->framesSent++; // 发送成功，帧计数加一
    return TRUE; // 推送成功
}

// 创建视频会话
static SessionData* make_video_session(VideoParameters* params, EncoderParameters* encoderParams) {
    GstElement* appsrc = gst_element_factory_make("appsrc", NULL);
    if (!appsrc) {
        log_message("创建 appsrc 元素失败");
        return NULL;
    }

    GstElement* encoder = gst_element_factory_make(encoderParams->type, NULL);
    if (!encoder) {
        log_message("创建编码器元素失败");
        gst_object_unref(appsrc);
        return NULL;
    }

    GstBin* videoBin = GST_BIN(gst_bin_new(NULL));
    GstCaps* caps = gst_caps_new_simple(params->codec,
        "width", G_TYPE_INT, params->width,
        "height", G_TYPE_INT, params->height,
        "framerate", GST_TYPE_FRACTION, params->fps, 1,
        NULL);
    
    g_object_set(appsrc, "caps", caps, NULL);
    gst_caps_unref(caps); // 释放 caps 内存
    gst_bin_add_many(videoBin, appsrc, encoder, NULL);
    
    if (!gst_element_link(appsrc, encoder)) { // 链接 appsrc 和编码器
        log_message("链接 appsrc 和编码器失败");
        gst_object_unref(videoBin);
        return NULL;
    }

    SessionData* session = session_new(0);
    session->input = GST_ELEMENT(videoBin);
    session->encoder = encoder;

    return session;
}

// 初始化 GStreamer 会话
void gstreamer_push_init(VideoParameters* params, EncoderParameters* encoderParams, InputThread* inputThread) {
    gst_init(NULL, NULL); // 初始化 GStreamer 库
    SessionData* videoSession = make_video_session(params, encoderParams); // 创建视频会话
    if (!videoSession) {
        log_message("初始化 GStreamer 会话失败");
        return;
    }

    inputThread->run = 1; 
    inputThread->thread = g_thread_new("PushVideoFrames", (GThreadFunc)push_video_frames, videoSession);
    log_message("GStreamer 会话初始化成功");
}

// 处理视频帧，推送到 GStreamer
void push_video_frames(SessionData* session) {
    while (session->run) {
        g_mutex_lock(&session->mutex);

        // 等待帧数据
        while (!session->frameReady) {
            g_cond_wait(&session->cond, &session->mutex);
        }

        // 获取帧数据并推送
        if (session->frameData != NULL) {
            gstreamer_send_frame(session, session->frameData, session->frameSize, session->pts);
        }

        session->frameReady = 0; // 重置帧准备状态
        free(session->frameData); // 释放帧数据
        session->frameData = NULL; // 重置帧数据指针
        g_mutex_unlock(&session->mutex);
    }
}

// 去初始化 GStreamer 会话
void gstreamer_push_deinit(SessionData* session, InputThread* inputThread) {
    inputThread->run = 0; // 设置线程状态为停止
    g_cond_signal(&session->cond); // 唤醒等待的线程
    g_thread_join(inputThread->thread); // 等待线程结束

    gst_app_src_end_of_stream(GST_APP_SRC(session->input)); 
    session_unref(session); // 释放会话数据
    log_message("GStreamer 会话去初始化成功");
}

// 主函数
int main() {
    VideoParameters params = { .width = 640, .height = 480, .fps = 30, .codec = "video/x-h265" };
    EncoderParameters encoderParams = { .type = "rtph265pay", .codec = "video/x-h265" };

    static InputThread inputThread; 
    static SessionData* videoSession = NULL;

    gstreamer_push_init(&params, &encoderParams, &inputThread);

    for (int i = 0; i < 100; i++) {
        uint8_t* frameData = (uint8_t*)malloc(params.width * params.height); // 创建帧数据
        if (!frameData) {
            log_message("分配帧数据失败");
            break; // 处理内存分配失败
        }
        
        memset(frameData, i % 256, params.width * params.height); // 填充帧数据
        push_frame_data(videoSession, frameData, params.width * params.height, g_get_monotonic_time()); // 设置当前帧数据
        g_usleep(1000000 / params.fps); // 控制帧率
    }

    int sentFrames = videoSession->framesSent; // 获取已发送的帧数
    printf("已发送帧数: %d\n", sentFrames);

    gstreamer_push_deinit(videoSession, &inputThread);
    return 0;
}