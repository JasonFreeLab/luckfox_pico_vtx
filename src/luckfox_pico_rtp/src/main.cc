#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <glib.h>

#include "luckfox_mpi.h"
#include "gst_push.h"

#define DEFAULT_IP "127.0.0.1" // 默认主机IP
#define DEFAULT_PORT 5000	   // 默认主机端口号
#define DEFAULT_WIDTH 1920	   // 默认视频宽度
#define DEFAULT_HEIGHT 1080	   // 默认视频高度
#define DEFAULT_FPS 90		   // 默认视频帧率
#define DEFAULT_ENCONDEC 1	   // 视频编码方式0:H264, 1:H265

int main(int argc, char *argv[])
{
	const char *host_ip = DEFAULT_IP;
	int host_port = DEFAULT_PORT;
	int video_width = DEFAULT_WIDTH;
	int video_height = DEFAULT_HEIGHT;
	int video_fps = DEFAULT_FPS;
	int video_encodec = DEFAULT_ENCONDEC;

	// 解析命令行参数
	int c;
	while ((c = getopt(argc, argv, "i:p:w:h:f:e:")) != -1)
	{
		switch (c)
		{
		case 'i':
			host_ip = optarg; // 设置主机IP
			break;
		case 'p':
			host_port = atoi(optarg); // 设置主机端口号
			break;
		case 'w':
			video_width = atoi(optarg); // 设置视频宽度
			break;
		case 'h':
			video_height = atoi(optarg); // 设置视频高度
			break;
		case 'f':
			video_fps = atoi(optarg); // 设置视频帧率
			break;
		case 'e':
			video_encodec = atoi(optarg); // 设置视频编码方式
			break;
		default:
			fprintf(stderr, "Usage: %s [-i host_ip] [-p host_port] [-w video_width] [-h video_height] [-f video_fps] [-e video_encodec(0:H264, 1:H265)]\n", argv[0]);
			fprintf(stderr, "For example: ./%s -i 127.0.0.1 -p 5000 -w 1920 -h 1080 -f 90 -e 1\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	system("RkLunch-stop.sh"); // 停止任何现有的RkLunch服务

	// rkaiq初始化
	RK_BOOL multi_sensor = RK_FALSE;							 // 多传感器标志
	const char *iq_dir = "/etc/iqfiles";						 // IQ文件目录
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL; // 工作模式设置
	SAMPLE_COMM_ISP_Init(0, hdr_mode, multi_sensor, iq_dir);	 // 初始化ISP
	SAMPLE_COMM_ISP_Run(0);										 // 运行ISP

	// rkmpi初始化
	if (RK_MPI_SYS_Init() != RK_SUCCESS)
	{
		RK_LOGE("rk mpi sys init fail!"); // 输出错误信息
		return -1;						  // 初始化失败，退出程序
	}

	// gst初始化
	GstPushInitParameter_S gst_push_init_parameter;

	gst_push_init_parameter.host_ip = (char *)host_ip;
	gst_push_init_parameter.host_port = host_port;
	gst_push_init_parameter.encodec_type = video_encodec ? EncondecType_E_H265 : EncondecType_E_H264;
	gst_push_init_parameter.fps = video_fps;
	if (gst_push_init(&gst_push_init_parameter) != RK_SUCCESS)
	{
		RK_LOGE("gst push init fail!"); // 输出错误信息
		return -1;						// 初始化失败，退出程序
	}

	// vi初始化
	vi_dev_init();							   // 初始化视频输入设备
	vi_chn_init(0, video_width, video_height); // 初始化视频输入通道

	// venc初始化
	RK_CODEC_ID_E enCodecType = video_encodec ? RK_VIDEO_ID_HEVC : RK_VIDEO_ID_AVC; // 设置编码类型
	venc_init(0, video_width, video_height, enCodecType);							// 初始化视频编码器

	// 绑定vi到venc
	MPP_CHN_S stSrcChn, stvencChn; // 声明源通道和编码通道结构

	stSrcChn.enModId = RK_ID_VI; // 设定视频输入模块ID
	stSrcChn.s32DevId = 0;		 // 设备ID
	stSrcChn.s32ChnId = 0;		 // 通道ID

	stvencChn.enModId = RK_ID_VENC; // 设定视频编码模块ID
	stvencChn.s32DevId = 0;			// 设备ID
	stvencChn.s32ChnId = 0;			// 通道ID

	if (RK_MPI_SYS_Bind(&stSrcChn, &stvencChn) != RK_SUCCESS) // 绑定视频输入和视频编码器
	{
		RK_LOGE("bind 0 ch venc failed"); // 输出错误信息
		return -1;						  // 绑定失败，退出程序
	}

	// 视频帧
	VENC_STREAM_S stFrame;
	stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S)); // 为编码包分配内存

	FrameData_S *frame;									// 帧数据变量
	frame = (FrameData_S *)malloc(sizeof(FrameData_S)); // 为编码包分配内存

	while (true)
	{
		// rtsp
		if (RK_MPI_VENC_GetStream(0, &stFrame, -1) == RK_SUCCESS) // 获取编码流
		{
			frame->buffer = (uint8_t *)RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk); // 获取视频帧数据
			frame->size = stFrame.pstPack->u32Len;										  // 获取帧大小
			frame->pts = stFrame.pstPack->u64PTS;										  // 获取PTS

			gst_push_data(frame); // 推送视频帧

			printf("fps = %.2f\n", (float)1000000 / (float)(TEST_COMM_GetNowUs() - frame->pts)); // 输出当前帧率
		}

		// 释放获取的帧
		if (RK_MPI_VENC_ReleaseStream(0, &stFrame) != RK_SUCCESS)
		{
			RK_LOGE("RK_MPI_VENC_ReleaseStream fail!\n"); // 输出错误信息
		}
	}

	// 解除绑定
	RK_MPI_SYS_UnBind(&stSrcChn, &stvencChn);

	RK_MPI_VI_DisableChn(0, 0); // 禁用视频输入通道
	RK_MPI_VI_DisableDev(0);	// 禁用视频输入设备

	SAMPLE_COMM_ISP_Stop(0); // 停止ISP

	RK_MPI_VENC_StopRecvFrame(0); // 停止接收编码帧
	RK_MPI_VENC_DestroyChn(0);	  // 销毁编码器通道

	free(stFrame.pstPack); // 释放编码包内存
	free(frame);		   // 释放帧数据包内存

	// 清理资源
	gst_push_deinit();

	RK_MPI_SYS_Exit(); // 退出RK MPI系统

	return 0; // 程序正常结束
}