#ifndef __GST_PUSH_H
#define __GST_PUSH_H

#include <stdint.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

// 获取编码后的视频帧
typedef struct
{
    uint8_t *buffer; // 视频帧数据
    size_t size;     // 视频帧大小
    guint64 pts;     // PTS（显示时间戳）
} FrameData_S;

// 定义队列的最大大小
#define QUEUE_SIZE 10

// 定义帧队列结构体
typedef struct
{
    FrameData_S frames[QUEUE_SIZE]; // 储存帧的数组
    int front;                      // 队头索引
    int rear;                       // 队尾索引
    int count;                      // 当前帧数
    pthread_mutex_t mutex;          // 互斥锁
    pthread_cond_t not_empty;       // 非空条件变量
    pthread_cond_t not_full;        // 非满条件变量
} FrameQueue;

typedef enum
{
    EncondecType_E_H264 = 0,
    EncondecType_E_H265 = 1
} EncondecType_E;

typedef struct
{
    char *host_ip;               // 目标主机IP
    uint16_t host_port;          // 目标主机端口号
    EncondecType_E encodec_type; // 视频帧编码类型
    guint64 fps;                 // FPS
} GstPushInitParameter_S;

/**
 * @brief 初始化帧队列
 *
 * @param queue 指向 FrameQueue 结构体的指针，表示要初始化的队列
 *
 * 本函数初始化指定的帧队列，设置队列的前指针、后指针和计数器为0，并初始化互斥锁和条件变量以供线程同步。
 */
void initQueue(FrameQueue *queue);

/**
 * @brief 将帧数据入队
 *
 * @param queue 指向 FrameQueue 结构体的指针，表示要操作的队列
 * @param frame FrameData_S 结构体，表示要入队的帧数据
 *
 * 本函数将帧数据插入到队列中。如果队列已满，则会等待直到有空间可用。
 */
void enqueue(FrameQueue *queue, FrameData_S frame);

/**
 * @brief 从队列中出队帧数据
 *
 * @param queue 指向 FrameQueue 结构体的指针，表示要操作的队列
 * @return FrameData_S 返回出队的帧数据
 *
 * 本函数从队列中移除并返回帧数据，如果队列为空，则等待直到有帧可用。
 */
FrameData_S dequeue(FrameQueue *queue);

/**
 * @brief 获取视频帧并将其推送到管道
 *
 * @param frame 指向 FrameData_S 结构体的指针，包含视频帧数据
 *
 * 本函数创建一个GStreamer缓冲区，填充视频帧数据，设置时间戳并将缓冲区推送到appsrc元素。
 */
void gst_push_data(FrameData_S *frame);

/**
 * @brief 初始化GStreamer管道
 *
 * @param gst_push_init_parameter 指向 GstPushInitParameter_S 结构体的指针，包含初始化参数
 * @return int 返回0表示成功，返回-1表示失败
 *
 * 本函数初始化GStreamer，创建所需的GStreamer元素，链接它们并设置属性。还会根据传入的帧率计算每帧的持续时间。
 */
int gst_push_init(GstPushInitParameter_S *gst_push_init_parameter);

/**
 * @brief 清理和释放GStreamer管道资源
 *
 * @return int 返回0表示成功
 *
 * 本函数发送EOS（End Of Stream）信号，处理消息，然后清理管道相关资源。
 */
int gst_push_deinit(void);

#endif //__GST_PUSH_H