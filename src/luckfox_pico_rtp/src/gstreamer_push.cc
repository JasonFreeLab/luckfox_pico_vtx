#include "gstreamer_push.h"


// 日志记录函数
void log_message(const char* message) {
    printf("%s\n", message);
}

// 推送帧数据到 GStreamer
gboolean gstreamer_push_frame_data(InputParameters* params) {
    // 推送数据到 GStreamer 流水线
    GstBuffer* buffer = gst_buffer_new_and_alloc(params->frameSize);
    if (!buffer) {
        log_message("创建新的 GstBuffer 失败");
        return FALSE; // 失败
    }

    gst_buffer_fill(buffer, 0, params->frameData, params->frameSize); // 填充帧数据
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, params->fps); // 设置帧率
    GST_BUFFER_PTS(buffer) = params->pts; // 设置PTS

    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(params->gst_element->appsrc), buffer);
    gst_buffer_unref(buffer); // 释放 GstBuffer

    if (ret != GST_FLOW_OK) {
        log_message("将缓冲区推送到 appsrc 时失败");
        return FALSE; // 推送失败
    }
    
    params->framesSent++; // 发送成功，帧计数加一
    return TRUE; // 推送成功
}

// 初始化 GStreamer 流水线
void gstreamer_push_init(InputParameters* params) {
    gst_init(NULL, NULL); // 初始化 GStreamer 库

    // 强制转换为 Gst_Element*，以避免编译器警告
    params->gst_element = (Gst_Element*)g_malloc0(sizeof(Gst_Element)); // 分配 Gst_Element 内存
    
    params->gst_element->appsrc = gst_element_factory_make("appsrc", NULL); // 创建 appsrc 元素
    if (!params->gst_element->appsrc) {
        log_message("未能创建 appsrc 元素");
        return;
    }

    params->gst_element->encoder = gst_element_factory_make(params->type, NULL); // 创建编码器
    if (!params->gst_element->encoder) {
        log_message("未能创建编码器");
        return;
    }

    params->gst_element->udpsink = gst_element_factory_make("udpsink", NULL); // 创建 udpsink 元素
    if (!params->gst_element->udpsink) {
        log_message("未能创建 udpsink 元素");
        return;
    }

    // 设置 udpsink 的主机和端口
    g_object_set(params->gst_element->udpsink, "host", params->g_host, "port", params->g_port, NULL); // 设置目标主机和端口

    GstBin* videoBin = GST_BIN(gst_bin_new("video-bin")); // 创建一个新的 GstBin 容器
    if (!videoBin) {
        log_message("创建 GstBin 失败");
        return; // 返回错误
    }

    GstCaps* caps = gst_caps_new_simple(params->codec,
        "width", G_TYPE_INT, params->width,
        "height", G_TYPE_INT, params->height,
        "framerate", GST_TYPE_FRACTION, params->fps, 1,
        NULL);
    
    g_object_set(params->gst_element->appsrc, "caps", caps, NULL); // 设置 appsrc 的能力
    gst_caps_unref(caps); // 释放 caps 内存

    // 将元素添加到视频 bin 中
    gst_bin_add_many(videoBin, params->gst_element->appsrc, params->gst_element->encoder, params->gst_element->udpsink, NULL);

    // 链接元素
    if (!gst_element_link(params->gst_element->appsrc, params->gst_element->encoder) ||
        !gst_element_link(params->gst_element->encoder, params->gst_element->udpsink)) {
        log_message("链接 appsrc 和编码器，或编码器和 udpsink 失败");
        gst_object_unref(videoBin);
        return;
    }

    // 启动 GStreamer 管道
    GstElement *pipeline = GST_ELEMENT(videoBin);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    
    log_message("GStreamer 会话初始化成功");
}

// 去初始化 GStreamer 会话
void gstreamer_push_deinit(InputParameters* params) {
    // 停止 GStreamer 管道
    GstElement *pipeline = GST_ELEMENT(gst_object_get_parent(GST_OBJECT(params->gst_element->appsrc)));
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL); 
        gst_object_unref(pipeline);
    }

    // 清理 Gst_Element
    if (params->gst_element) {
        g_free(params->gst_element);
    }

    log_message("GStreamer 会话去初始化成功");
}

/* 主函数
int main_int() {
    InputParameters params = {
        .g_host = "127.0.0.1", // 目标主机
        .g_port = 5000,        // 目标端口
        .width = 1280,         // 视频宽度
        .height = 720,         // 视频高度
        .fps = 30,             // 帧率
        .codec = "video/x-h265",      // 编码格式
        .type = "rtph265pay",       // 编码器类型
        .frameData = NULL,           // 初始化为NULL，稍后分配
        .frameSize = 1280 * 720 * 3, // 设置帧大小，假设为RGB格式
        .pts = 0,                   // 初始化PTS
        .framesSent = 0,            // 已发送的帧计数
        .gst_element = NULL          // 初始化gst_element为NULL
    };

    // 注意！这里应有一个实际的帧数据格式判定
    params.frameData = (uint8_t *)malloc(params.frameSize);
    if (!params.frameData) {
        log_message("分配帧数据失败");
        return -1; // 处理分配失败
    } 

    gstreamer_push_init(&params); // 初始化 GStreamer 流水线

    // 处理并推送视频帧
    guint64 frame_duration = 1000000 / params.fps; // 计算每帧间隔（微秒）

    for (int i = 0; i < 100; i++) {
        memset(params.frameData, i % 256, params.frameSize); // 填充帧数据
        params.pts = gst_util_uint64_scale(i, GST_SECOND, params.fps); // 设置正确的 PTS
        if (!push_frame_data(&params)) {
            log_message("推送帧数据失败");
            break;
        }
        
        g_usleep(frame_duration); // 控制帧率
    }

    printf("已发送帧数: %d\n", params.framesSent); // 输出已发送的帧数

    // 在这里释放帧数据
    free(params.frameData);

    gstreamer_push_deinit(&params); // 去初始化 GStreamer 会话
    return 0;
}
*/