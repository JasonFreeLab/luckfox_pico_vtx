#include <stdio.h>	// 标准输入输出库，用于printf和fprintf等函数
#include <stdlib.h> // 提供动态内存分配、随机数生成、程序控制等功能
#include <string.h> // 提供字符串处理功能，如strcmp
#include <unistd.h> // 提供对POSIX操作系统API的访问，usleep用于线程暂停
#include <time.h>	// 提供时间处理功能，包括nanosleep等

#include "luckfox_mpi.h" // 自定义头文件，可能包含与多媒体处理相关的函数
#include "gst_push.h"	 // 自定义头文件，可能包含与GStreamer推送数据相关的函数

// 定义一些常量，用于设置默认程序参数
#define DEFAULT_IP "127.0.0.1" // 默认主机IP地址
#define DEFAULT_PORT 5602	   // 默认主机端口号
#define DEFAULT_WIDTH 1920	   // 默认视频宽度
#define DEFAULT_HEIGHT 1080	   // 默认视频高度
#define DEFAULT_FPS 90		   // 默认视频帧率
#define DEFAULT_ENCONDEC 1	   // 默认视频编码方式(0为H264, 1为H265)
#define DEFAULT_BITRATE 2	   // 定义比特率常量，设置为 2Mbps
#define DEFAULT_GOP 15		   // 定义GOP常量，设置为 15

/**
 * @brief 获取当前时间（微秒）
 *
 * @return RK_U64 返回当前时间，单位为微秒
 */
RK_U64 TEST_COMM_GetNowUs(void)
{
	struct timespec time = {0, 0};										// 定义一个 timespec 结构体用于存储时间
	clock_gettime(CLOCK_MONOTONIC, &time);								// 获取当前的单调时钟时间
																		// 将秒转换为微秒并加上纳秒转换为微秒的结果，返回微秒级的当前时间
	return (RK_U64)time.tv_sec * 1000000 + (RK_U64)time.tv_nsec / 1000; /* microseconds */
}

/**
 * @brief 程序的使用说明
 *
 * @param program_name 程序名称字符串
 */
void display_usage(const char *program_name)
{
	fprintf(stderr, "Usage: %s [-i host_ip] [-p host_port] [-w video_width] [-h video_height] [-f video_fps] [-e video_encodec(0:H264, 1:H265)] [-b video_bitrate] [-g video_gop]\n", program_name);
	fprintf(stderr, "For example: %s -i 127.0.0.1 -p 5602 -w 1920 -h 1080 -f 90 -e 1 -b 2 -g 15\n", program_name);
}

/**
 * @brief 主程序入口
 *
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 返回0表示程序正常结束，非0表示程序异常结束
 */
int main(int argc, char *argv[])
{
	const char *host_ip = DEFAULT_IP;		 // 主机IP的初始值
	uint16_t host_port = DEFAULT_PORT;		 // 主机端口的初始值
	uint16_t video_width = DEFAULT_WIDTH;	 // 视频宽度的初始值
	uint16_t video_height = DEFAULT_HEIGHT;	 // 视频高度的初始值
	uint8_t video_fps = DEFAULT_FPS;		 // 视频帧率的初始值
	bool video_encodec = DEFAULT_ENCONDEC;	 // 视频编码方式的初始值
	uint8_t video_bitrate = DEFAULT_BITRATE; // 视频编码比特率的初始值
	uint8_t video_gop = DEFAULT_GOP;		 // 视频图像组大小的初始值

	// 解析命令行参数
	int c;
	while ((c = getopt(argc, argv, "i:p:w:h:f:e:b:g:")) != -1) // 逐个获取命令行选项
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
		case 'b':
			video_bitrate = atoi(optarg); // 设置视频编码比特率
			break;
		case 'g':
			video_gop = atoi(optarg); // 设置视频图像组大小
			break;
		default:
			display_usage(argv[0]); // 若无效选项，显示使用说明
			exit(EXIT_FAILURE);		// 退出程序
		}
	}

	// 检查是否只有 -h 或 --help 选项
	if (argc == 1 || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)))
	{
		display_usage(argv[0]); // 显示使用说明
		exit(EXIT_SUCCESS);		// 正常退出
	}

	// system("RkLunch-stop.sh"); // 停止任何现有的RkLunch服务

	// rk_aiq初始化
	RK_BOOL multi_sensor = RK_FALSE;							 // 多传感器标志
	const char *iq_dir = "/etc/iqfiles";						 // IQ文件目录
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL; // 工作模式设置
	SAMPLE_COMM_ISP_Init(0, hdr_mode, multi_sensor, iq_dir);	 // 初始化图像信号处理（ISP）
	SAMPLE_COMM_ISP_Run(0);										 // 运行ISP

	// rkmpi初始化
	if (RK_MPI_SYS_Init() != RK_SUCCESS) // 初始化多媒体处理接口
	{
		RK_LOGE("rk mpi sys init fail!"); // 输出错误信息
		return -1;						  // 初始化失败，退出程序
	}

	// gst初始化
	GstPushInitParameter_S gst_push_init_parameter; // 创建GStreamer推送初始化参数结构

	// 设置GStreamer推送参数
	gst_push_init_parameter.host_ip = (char *)host_ip;												  // 推送主机IP
	gst_push_init_parameter.host_port = host_port;													  // 推送主机端口号
	gst_push_init_parameter.encodec_type = video_encodec ? EncondecType_E_H265 : EncondecType_E_H264; // 编码方式选择
	gst_push_init_parameter.fps = video_fps;														  // 视频帧率

	if (gst_push_init(&gst_push_init_parameter) != RK_SUCCESS) // 初始化GStreamer推送
	{
		RK_LOGE("gst push init fail!"); // 输出错误信息
		return -1;						// 初始化失败，退出程序
	}

	// vi初始化
	vi_dev_init();							   // 初始化视频输入设备
	vi_chn_init(0, video_width, video_height); // 初始化视频输入通道

	// vpss init
	// vpss_init(0, video_width, video_height);

	// venc初始化
	RK_CODEC_ID_E enCodecType = video_encodec ? RK_VIDEO_ID_HEVC : RK_VIDEO_ID_AVC;			   // 设置编码类型
	venc_init(0, video_width, video_height, enCodecType, video_bitrate, video_fps, video_gop); // 初始化视频编码器

	// 绑定vi到venc
	MPP_CHN_S stSrcChn, stvpssChn, stvencChn; // 声明源通道和编码通道结构

	// 设置源通道参数
	stSrcChn.enModId = RK_ID_VI; // 视频输入模块ID
	stSrcChn.s32DevId = 0;		 // 设备ID
	stSrcChn.s32ChnId = 0;		 // 通道ID

	// stvpssChn.enModId = RK_ID_VPSS;
	// stvpssChn.s32DevId = 0;
	// stvpssChn.s32ChnId = 0;

	// 设置编码通道参数
	stvencChn.enModId = RK_ID_VENC; // 视频编码模块ID
	stvencChn.s32DevId = 0;			// 设备ID
	stvencChn.s32ChnId = 0;			// 通道ID

	// 绑定视频输入和视频编码器
	// if (RK_MPI_SYS_Bind(&stSrcChn, &stvpssChn) != RK_SUCCESS)
	// {
	// 	RK_LOGE("bind vi0 vpss0 failed"); // 输出错误信息
	// 	return -1;						  // 绑定失败，退出程序
	// }
	// 绑定视频输入和视频编码器
	if (RK_MPI_SYS_Bind(&stSrcChn, &stvencChn) != RK_SUCCESS)
	{
		RK_LOGE("bind vpss0 venc0 failed"); // 输出错误信息
		return -1;							// 绑定失败，退出程序
	}

	// 视频帧
	VENC_STREAM_S stFrame;										  // 声明编码流结构
	stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S)); // 为编码包分配内存

	FrameData_S frame; // 声明帧数据变量

	while (true) // 无限循环处理视频流
	{
		// 获取编码流
		if (RK_MPI_VENC_GetStream(0, &stFrame, -1) == RK_SUCCESS)
		{
			// 获取视频帧数据
			frame.buffer = (uint8_t *)RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk); // 获取视频帧数据
			frame.size = stFrame.pstPack->u32Len;										 // 获取帧大小
			frame.pts = stFrame.pstPack->u64PTS;										 // 获取PTS

			gst_push_data(&frame); // 推送视频帧

			printf("fps = %.2f\n", (float)1000000 / (float)(TEST_COMM_GetNowUs() - frame->pts)); // 输出当前帧率
		}

		// 释放编码流
		if (RK_MPI_VENC_ReleaseStream(0, &stFrame) != RK_SUCCESS)
		{
			RK_LOGE("RK_MPI_VENC_ReleaseStream fail!\n"); // 输出错误信息
		}
	}

	// 解除绑定输入通道和编码器
	RK_MPI_SYS_UnBind(&stSrcChn, &stvencChn);

	RK_MPI_VI_DisableChn(0, 0); // 禁用视频输入通道
	RK_MPI_VI_DisableDev(0);	// 禁用视频输入设备

	SAMPLE_COMM_ISP_Stop(0); // 停止ISP

	RK_MPI_VENC_StopRecvFrame(0); // 停止接收编码帧
	RK_MPI_VENC_DestroyChn(0);	  // 销毁编码器通道

	free(stFrame.pstPack); // 释放编码包内存

	// 清理资源
	RK_LOGE("gst push deinit.\n");
	gst_push_deinit(); // 反初始化GStreamer推送

	RK_MPI_SYS_Exit(); // 退出RK MPI系统

	return 0; // 程序正常结束
}