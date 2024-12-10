#include <X11/Xlib.h>               // 包含X11库的头文件，用于创建窗口和处理显示。
#include <glib.h>                   // 包含glib库的头文件，提供基本数据结构和功能。
#include <gst/gst.h>                // 包含GStreamer库的头文件，用于处理多媒体流。
#include <gst/video/videooverlay.h> // 包含处理视频叠加的GStreamer头文件。
#include <stdio.h>                  // 包含标准输入输出库，提供printf等函数的功能。
#include <string.h>                 // 包含字符串处理库，提供字符串操作的功能。

// 定义一个全局变量用于窗口
Window win;

/**
 * @brief 初始化GStreamer管道以播放通过UDP接收的H265视频流。
 *
 * @details 此函数创建一个GStreamer管道，包括源、解封装器、解析器、解码器、转换器和输出显示效果器。
 * 它将数据从UDP端口5600接收，并通过X11窗口播放视频流。
 *
 * @return 无输出参数。
 */
void initGst2(void)
{
    // 定义GStreamer元素，包括管道和各个组件
    GstElement *gst_pipeline;
    GstElement *gst_src;
    GstElement *gst_depayloader;
    GstElement *gst_parser;
    GstElement *gst_decoder;
    GstElement *gst_conv;
    GstElement *gst_sink;

    // 创建一个新的GStreamer管道，用于后续的元素连接
    gst_pipeline = gst_pipeline_new("xvoverlay");

    // 创建UDP源，监听127.0.0.1:5600
    gst_src = gst_element_factory_make("udpsrc", "source");
    g_object_set(G_OBJECT(gst_src), "port", 5600, NULL); // 设置UDP端口为5600
    g_object_set(G_OBJECT(gst_src), "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", "encoding-name", G_TYPE_STRING, "H265", NULL), NULL);

    // 创建RTP解封装器以从RTP包中提取H265数据
    gst_depayloader = gst_element_factory_make("rtph265depay", "depayloader");

    // 创建H265解析器，用于解析H265流
    gst_parser = gst_element_factory_make("h265parse", NULL);

    // 创建H265解码器，将H265数据解码为原始视频格式
    gst_decoder = gst_element_factory_make("avdec_h265", NULL);

    // 创建视频转换元素，以处理视频格式转换
    gst_conv = gst_element_factory_make("videoconvert", NULL);

    // 创建视频显示元素，使用X11显示接收视频
    gst_sink = gst_element_factory_make("xvimagesink", "sink");

    // 将所有元素添加到管道中
    gst_bin_add_many(GST_BIN(gst_pipeline), gst_src, gst_depayloader, gst_parser, gst_decoder, gst_conv, gst_sink, NULL);

    // 连接所有元素，形成处理链路
    if (gst_element_link_many(gst_src, gst_depayloader, gst_parser, gst_decoder, gst_conv, gst_sink, NULL) == 0)
    {
        printf("gst_element_link_many error!\r\n"); // 如果链接失败，打印错误信息
    }

    // 将X11窗口句柄绑定到GStreamer的sink元素上，使视频能正确显示在窗口中
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(gst_sink), win);

    // 改变管道的状态为PLAYING，开始播放
    GstStateChangeReturn sret = gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
    printf("set gst playing. sret=%d\r\n", sret);
}

/**
 * @brief 主程序入口，初始化GStreamer并创建一个X11窗口。
 *
 * @details 此函数负责初始化GStreamer库、打开X11显示以及创建和管理窗口；
 * 最后，它等待用户释放按钮事件以销毁窗口并关闭显示。
 *
 * @param argc 输入参数，命令行参数数量。
 * @param argv 输入参数，命令行参数数组。
 *
 * @return 返回值为0表示成功，返回值为1表示失败。
 */
int main(int argc, char *argv[])
{
    /* 初始化GStreamer库 */
    gst_init(&argc, &argv); // 初始化GStreamer库，处理任何命令行参数

    // 打开一个显示，连接到默认的X显示
    Display *dsp = XOpenDisplay(NULL);
    if (!dsp) // 检查是否成功打开显示
    {
        return 1; // 如果失败，则返回错误代码
    }

    // 获取默认屏幕的信息
    int screenNumber = DefaultScreen(dsp);
    unsigned long white = WhitePixel(dsp, screenNumber); // 获取白色像素值
    unsigned long black = BlackPixel(dsp, screenNumber); // 获取黑色像素值

    // 创建一个简单的窗口
    win = XCreateSimpleWindow(dsp,
                              DefaultRootWindow(dsp), // 根窗口
                              50, 50,                 // 窗口的位置
                              800, 600,               // 窗口的大小
                              0, black,               // 窗口边框颜色为黑色
                              white);                 // 窗口背景颜色为白色

    XMapWindow(dsp, win); // 显示窗口

    long eventMask = StructureNotifyMask; // 设置事件掩码以捕捉窗口结构事件
    XSelectInput(dsp, win, eventMask);    // 选择输入事件

    XEvent evt; // 定义X事件
    do
    {
        XNextEvent(dsp, &evt); // 等待并处理下一个事件
    } while (evt.type != MapNotify); // 等待窗口被映射结束

    // 创建具体的视频处理管道，并绑定刚才的创建的窗口
    initGst2();

    // 等待直到用户释放鼠标按钮
    do
    {
        XNextEvent(dsp, &evt); // 一直处理事件，直到捕捉到按钮释放事件
    } while (evt.type != ButtonRelease);

    // 销毁窗口和关闭显示
    XDestroyWindow(dsp, win);
    XCloseDisplay(dsp);

    return 0; // 返回0表示程序成功结束
}