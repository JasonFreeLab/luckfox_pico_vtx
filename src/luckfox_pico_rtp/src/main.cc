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

#define VIDEO_WIDTH 1280 // 视频宽度
#define VIDEO_HEIGHT 720 // 视频高度
#define VIDEO_FPS 120	 // 视频帧率

int main(int argc, char *argv[])
{
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
	if (gst_push_init() != RK_SUCCESS)
	{
		RK_LOGE("gst push init fail!"); // 输出错误信息
		return -1;						// 初始化失败，退出程序
	}

	// vi初始化
	vi_dev_init();							   // 初始化视频输入设备
	vi_chn_init(0, VIDEO_WIDTH, VIDEO_HEIGHT); // 初始化视频输入通道

	// venc初始化
	RK_CODEC_ID_E enCodecType = RK_VIDEO_ID_HEVC;		  // 编码类型设置为H.265
	venc_init(0, VIDEO_WIDTH, VIDEO_HEIGHT, enCodecType); // 初始化视频编码器

	// 绑定vi到venc
	MPP_CHN_S stSrcChn, stvencChn; // 声明源通道和编码通道结构

	stSrcChn.enModId = RK_ID_VI; // 设定视频输入模块ID
	stSrcChn.s32DevId = 0;		 // 设备ID
	stSrcChn.s32ChnId = 0;		 // 通道ID

	stvencChn.enModId = RK_ID_VENC; // 设定视频编码模块ID
	stvencChn.s32DevId = 0;			// 设备ID
	stvencChn.s32ChnId = 0;			// 通道ID

	if (RK_MPI_SYS_Bind(&stSrcChn, &stvencChn) != RK_SUCCESS)
	{									  // 绑定视频输入和视频编码器
		RK_LOGE("bind 0 ch venc failed"); // 输出错误信息
		return -1;						  // 绑定失败，退出程序
	}

	// guint64 frame_duration = 1000000 / VIDEO_FPS; // 计算每帧间隔（微秒）
	RK_U64 nowUs = 0;	// 当前时间（微秒）

	// h265帧
	VENC_STREAM_S stFrame;
	stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S)); // 为编码包分配内存

	FrameData frame; // 帧数据变量

	while (true)
	{
		// rtsp
		if (RK_MPI_VENC_GetStream(0, &stFrame, -1) == RK_SUCCESS)
		{																				 // 获取编码流
			frame.buffer = (uint8_t *)RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk); // 获取视频帧数据
			frame.size = stFrame.pstPack->u32Len;										 // 获取帧大小
			frame.pts = stFrame.pstPack->u64PTS;										 // 获取PTS

			gst_push_data(&frame);
		}

		// 释放帧
		if (RK_MPI_VENC_ReleaseStream(0, &stFrame) != RK_SUCCESS)
		{														  // 释放获取的帧
			RK_LOGE("RK_MPI_VENC_ReleaseStream fail!\n"); // 输出错误信息
		}

		printf("fps = %.2f\n", (float)1000000 / (float)(frame.pts - nowUs)); // 输出当前帧率

		nowUs = frame.pts;

		// g_usleep(frame_duration); // 控制帧率
	}

	// 解除绑定
	RK_MPI_SYS_UnBind(&stSrcChn, &stvencChn);

	RK_MPI_VI_DisableChn(0, 0); // 禁用视频输入通道
	RK_MPI_VI_DisableDev(0);	// 禁用视频输入设备

	SAMPLE_COMM_ISP_Stop(0); // 停止ISP

	RK_MPI_VENC_StopRecvFrame(0); // 停止接收编码帧
	RK_MPI_VENC_DestroyChn(0);	  // 销毁编码器通道

	free(stFrame.pstPack); // 释放编码包内存

	// 清理资源
	gst_push_deinit();

	RK_MPI_SYS_Exit(); // 退出RK MPI系统

	return 0; // 程序正常结束
}