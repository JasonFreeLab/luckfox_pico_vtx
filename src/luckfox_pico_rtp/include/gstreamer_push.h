#ifndef GSTREAMER_PUSH_H
#define GSTREAMER_PUSH_H

// 函数声明
void gstreamer_push_init(VideoParameters* params, EncoderParameters* encoderParams, InputThread* inputThread); // 初始化函数
void gstreamer_push_deinit(SessionData* session);                             // 去初始化函数
gboolean gstreamer_send_frame(SessionData* session, uint8_t* frameData, guint frameSize, guint64 pts); // 发送视频帧函数
void push_video_frames(SessionData* session); // 用于推送视频帧的函数
void set_frame_data(SessionData* session, uint8_t* frameData, guint frameSize, guint64 pts); // 设置帧数据的函数
int check_frames_sent(SessionData* session); // 检查已发送帧的数量

#endif // GSTREAMER_PUSH_H