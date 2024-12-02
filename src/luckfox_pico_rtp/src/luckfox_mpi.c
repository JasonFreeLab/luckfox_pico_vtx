#include <stdio.h>		 // 引入标准输入输出库，支持打印功能
#include "luckfox_mpi.h" // 引入专用的库，用于操作多媒体接口

#define BITRATE (2 * 1024) // 定义比特率常量，设置为 2KB/s

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
 * @brief 初始化视频设备
 *
 * @return int 返回0表示成功，-1表示失败
 */
int vi_dev_init(void)
{
	printf("%s\n", __func__); // 打印当前函数的名称
	int ret = 0;			  // 初始化返回值
	int devId = 0;			  // 视频设备 ID, 此处固定为 0
	int pipeId = devId;		  // 管道 ID 设为同设备 ID

	VI_DEV_ATTR_S stDevAttr;					// 定义视频设备属性结构体
	VI_DEV_BIND_PIPE_S stBindPipe;				// 定义设备绑定管道结构体
	memset(&stDevAttr, 0, sizeof(stDevAttr));	// 清零设备属性结构体
	memset(&stBindPipe, 0, sizeof(stBindPipe)); // 清零绑定管道结构体

	// 0. 获取设备配置状态
	ret = RK_MPI_VI_GetDevAttr(devId, &stDevAttr);
	if (ret == RK_ERR_VI_NOT_CONFIG) // 检查设备是否已配置
	{
		// 0-1. 配置设备
		ret = RK_MPI_VI_SetDevAttr(devId, &stDevAttr); // 设置设备属性
		if (ret != RK_SUCCESS)						   // 检查设置是否成功
		{
			printf("RK_MPI_VI_SetDevAttr %x\n", ret); // 打印错误码
			return -1;								  // 返回失败
		}
	}
	else
	{
		printf("RK_MPI_VI_SetDevAttr already\n"); // 设备已配置
	}

	// 1. 获取设备使能状态
	ret = RK_MPI_VI_GetDevIsEnable(devId);
	if (ret != RK_SUCCESS) // 检查设备是否已使能
	{
		// 1-2. 启用设备
		ret = RK_MPI_VI_EnableDev(devId);
		if (ret != RK_SUCCESS) // 检查启用是否成功
		{
			printf("RK_MPI_VI_EnableDev %x\n", ret); // 打印错误码
			return -1;								 // 返回失败
		}
		// 1-3. 绑定设备/管道
		stBindPipe.u32Num = pipeId;							// 设置绑定管道数量
		stBindPipe.PipeId[0] = pipeId;						// 绑定管道 ID
		ret = RK_MPI_VI_SetDevBindPipe(devId, &stBindPipe); // 设置设备绑定管道
		if (ret != RK_SUCCESS)								// 检查设置是否成功
		{
			printf("RK_MPI_VI_SetDevBindPipe %x\n", ret); // 打印错误码
			return -1;									  // 返回失败
		}
	}
	else
	{
		printf("RK_MPI_VI_EnableDev already\n"); // 设备已启用
	}

	return 0; // 返回成功
}

/**
 * @brief 初始化视频通道
 *
 * @param channelId 通道 ID，类型为 int
 * @param width 通道图像宽度，类型为 int
 * @param height 通道图像高度，类型为 int
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int vi_chn_init(int channelId, int width, int height)
{
	int ret;		 // 用于存储返回值
	int buf_cnt = 2; // 定义缓冲区数量为 2

	// 视频通道初始化
	VI_CHN_ATTR_S vi_chn_attr;					  // 定义视频通道属性结构体
	memset(&vi_chn_attr, 0, sizeof(vi_chn_attr)); // 清零通道属性结构体

	vi_chn_attr.stIspOpt.u32BufCount = buf_cnt;						// 设置缓冲区数量
	vi_chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF; // 设置内存类型为 DMA 缓冲区
	vi_chn_attr.stSize.u32Width = width;							// 设置图像宽度
	vi_chn_attr.stSize.u32Height = height;							// 设置图像高度
	vi_chn_attr.enPixelFormat = RK_FMT_YUV420SP;					// 设置像素格式为 YUV420SP
	vi_chn_attr.enCompressMode = COMPRESS_MODE_NONE;				// 设置压缩模式为无压缩
	vi_chn_attr.u32Depth = 2;										// 设置深度为 2

	// 设置通道属性并启用通道
	ret = RK_MPI_VI_SetChnAttr(0, channelId, &vi_chn_attr); // 设置通道属性
	ret |= RK_MPI_VI_EnableChn(0, channelId);				// 启用通道
	if (ret)												// 检查是否有错误
	{
		printf("ERROR: create VI error! ret=%d\n", ret); // 打印错误信息
		return ret;										 // 返回错误码
	}

	return ret; // 返回成功
}

/**
 * @brief 初始化视频编码通道
 *
 * @param chnId 编码通道 ID，类型为 int
 * @param width 编码图像宽度，类型为 int
 * @param height 编码图像高度，类型为 int
 * @param enType 编码类型，类型为 RK_CODEC_ID_E
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType)
{
	printf("%s\n", __func__); // 打印当前函数的名称

	VENC_RECV_PIC_PARAM_S stRecvParam;			 // 定义接收参数结构体
	VENC_CHN_ATTR_S stAttr;						 // 定义编码通道属性结构体
	memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S)); // 清零编码通道属性结构体

	// 根据编码类型设置相应的属性
	if (enType == RK_VIDEO_ID_AVC)
	{
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR; // 设置为 H.264 CBR 模式
		stAttr.stRcAttr.stH264Cbr.u32BitRate = BITRATE;	 // 设置比特率
		stAttr.stRcAttr.stH264Cbr.u32Gop = 1;			 // 设置 GOP 大小
	}
	else if (enType == RK_VIDEO_ID_HEVC)
	{
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR; // 设置为 H.265 CBR 模式
		stAttr.stRcAttr.stH265Cbr.u32BitRate = BITRATE;	 // 设置比特率
		stAttr.stRcAttr.stH265Cbr.u32Gop = 60;			 // 设置 GOP 大小
	}
	else if (enType == RK_VIDEO_ID_MJPEG)
	{
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR; // 设置为 MJPEG CBR 模式
		stAttr.stRcAttr.stMjpegCbr.u32BitRate = BITRATE;  // 设置比特率
	}

	// 设置编码通道的其他属性
	stAttr.stVencAttr.enType = enType;				   // 设置编码类型
	stAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP; // 设置像素格式为 YUV420SP
	if (enType == RK_VIDEO_ID_AVC)
		stAttr.stVencAttr.u32Profile = H264E_PROFILE_HIGH; // 设置 H.264 的信号质量

	// 设置图像尺寸和缓冲区信息
	stAttr.stVencAttr.u32PicWidth = width;				   // 设置图片宽度
	stAttr.stVencAttr.u32PicHeight = height;			   // 设置图片高度
	stAttr.stVencAttr.u32VirWidth = width;				   // 设置虚拟宽度
	stAttr.stVencAttr.u32VirHeight = height;			   // 设置虚拟高度
	stAttr.stVencAttr.u32StreamBufCnt = 2;				   // 设置流缓冲区数量
	stAttr.stVencAttr.u32BufSize = width * height * 3 / 2; // 设置缓冲区大小
	stAttr.stVencAttr.enMirror = MIRROR_NONE;			   // 设置镜像模式

	// 创建编码通道
	RK_MPI_VENC_CreateChn(chnId, &stAttr);

	memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S)); // 清零接收参数结构体
	stRecvParam.s32RecvPicNum = -1;							// 设置接收图片数量为-1（表示不限制）
	RK_MPI_VENC_StartRecvFrame(chnId, &stRecvParam);		// 开始接收编码帧

	return 0; // 返回成功
}
