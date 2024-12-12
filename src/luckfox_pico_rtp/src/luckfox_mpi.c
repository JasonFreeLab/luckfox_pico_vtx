#include <stdio.h>		 // 引入标准输入输出库，支持打印功能
#include "luckfox_mpi.h" // 引入专用的库，用于操作多媒体接口

/**
 * @brief 初始化视频设备
 *
 * @return int 返回0表示成功，-1表示失败
 */
int vi_dev_init(void)
{
	int ret = 0;		// 初始化返回值
	int devId = 0;		// 视频设备 ID, 此处固定为 0
	int pipeId = devId; // 管道 ID 设为同设备 ID

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
 * @param channelId 通道 ID，类型为 uint8_t
 * @param width 通道图像宽度，类型为 uint16_t
 * @param height 通道图像高度，类型为 uint16_t
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int vi_chn_init(uint8_t channelId, uint16_t width, uint16_t height)
{
	int ret; // 用于存储返回值

	// 视频通道初始化
	VI_CHN_ATTR_S vi_chn_attr;						// 定义视频通道属性结构体
	memset(&vi_chn_attr, 0, sizeof(VI_CHN_ATTR_S)); // 清零通道属性结构体

	vi_chn_attr.stIspOpt.u32BufCount = 2;							// 设置缓冲区数量 2
	vi_chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF; // 设置内存类型为 DMA 缓冲区
	vi_chn_attr.stSize.u32Width = width;							// 设置图像宽度
	vi_chn_attr.stSize.u32Height = height;							// 设置图像高度
	vi_chn_attr.enPixelFormat = RK_FMT_YUV420SP;					// 设置像素格式为 YUV420SP
	vi_chn_attr.enCompressMode = COMPRESS_MODE_NONE;				// 设置压缩模式为无压缩
	vi_chn_attr.u32Depth = 1;										// 设置深度为 1  //0, get fail; 1 - u32BufCount, can get, if bind to other device, must be < u32BufCount

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
 * @brief 初始化 VPSS（视频前端支撑子系统）组
 *
 * @param VpssChn VPSS 通道，类型为 uint8_t
 * @param width 图像宽度，类型为 uint16_t
 * @param height 图像高度，类型为 uint16_t
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int vpss_init(uint8_t VpssChn, uint16_t width, uint16_t height)
{
	VPSS_GRP_ATTR_S stGrpVpssAttr;						// 定义 VPSS 组属性结构体
	memset(&stGrpVpssAttr, 0, sizeof(VPSS_GRP_ATTR_S)); // 清零组属性结构体

	// 设置组的属性
	stGrpVpssAttr.u32MaxW = 4096;					   // 设置组的最大宽度
	stGrpVpssAttr.u32MaxH = 4096;					   // 设置组的最大高度
	stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;	   // 设置像素格式为 YUV420SP
	stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;	   // 设置源帧率为 -1（未限制）
	stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;	   // 设置目标帧率为 -1（未限制）
	stGrpVpssAttr.enCompressMode = COMPRESS_MODE_NONE; // 设置压缩模式为无压缩

	VPSS_CHN_ATTR_S stVpssChnAttr;						// 定义 VPSS 通道属性结构体
	memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S)); // 清零通道属性结构体

	// 设置通道的属性
	stVpssChnAttr.enChnMode = VPSS_CHN_MODE_USER;	   // 设置通道模式为用户模式
	stVpssChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8; // 设置动态范围为 SDR8
	stVpssChnAttr.enPixelFormat = RK_FMT_YUV420SP;	   // 设置像素格式为 YUV420SP
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;	   // 设置源帧率为 -1（未限制）
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;	   // 设置目标帧率为 -1（未限制）
	stVpssChnAttr.u32Width = width;					   // 设置图像宽度
	stVpssChnAttr.u32Height = height;				   // 设置图像高度
	stVpssChnAttr.enCompressMode = COMPRESS_MODE_NONE; // 设置压缩模式为无压缩

	int s32Ret;		// 用于存储返回值
	int s32Grp = 0; // 组 ID，初始化为 0

	// 创建 VPSS 组并检查返回值
	s32Ret = RK_MPI_VPSS_CreateGrp(s32Grp, &stGrpVpssAttr);
	if (s32Ret != RK_SUCCESS) // 检查创建是否成功
	{
		return s32Ret; // 返回错误码
	}
	// 设置通道属性并检查返回值
	s32Ret = RK_MPI_VPSS_SetChnAttr(s32Grp, VpssChn, &stVpssChnAttr);
	if (s32Ret != RK_SUCCESS) // 检查设置是否成功
	{
		return s32Ret; // 返回错误码
	}
	// 启用通道并检查返回值
	s32Ret = RK_MPI_VPSS_EnableChn(s32Grp, VpssChn);
	if (s32Ret != RK_SUCCESS) // 检查启用是否成功
	{
		return s32Ret; // 返回错误码
	}
	// 启动 VPSS 组并检查返回值
	s32Ret = RK_MPI_VPSS_StartGrp(s32Grp);
	if (s32Ret != RK_SUCCESS) // 检查启动是否成功
	{
		return s32Ret; // 返回错误码
	}

	return s32Ret; // 返回成功标志
}

/**
 * @brief 初始化视频编码通道
 *
 * @param chnId 编码通道 ID，类型为 uint8_t
 * @param width 编码图像宽度，类型为 uint16_t
 * @param height 编码图像高度，类型为 uint16_t
 * @param enType 编码类型，类型为 RK_CODEC_ID_E
 * @param bitrate 编码比特率，类型为 uint8_t
 * @param fps 编码帧率，类型为 uint8_t
 * @param gop 图像组大小，类型为 uint8_t
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int venc_init(uint8_t chnId, uint16_t width, uint16_t height, RK_CODEC_ID_E enType, uint8_t bitrate, uint8_t fps, uint8_t gop)
{
	VENC_CHN_ATTR_S stAttr;						 // 定义编码通道属性结构体
	memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S)); // 清零编码通道属性结构体

	// 根据编码类型设置相应的属性
	if (enType == RK_VIDEO_ID_AVC) // 如果编码类型为 H.264
	{
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;				 // 设置为 H.264 自适应比特率模式
		stAttr.stRcAttr.stH264Avbr.u32BitRate = bitrate * 1024;			 // 设置比特率
		stAttr.stRcAttr.stH264Avbr.u32MaxBitRate = (bitrate + 1) * 1024; // 设置最大比特率
		stAttr.stRcAttr.stH264Avbr.u32MinBitRate = 0;					 // 设置最小比特率为 0
		stAttr.stRcAttr.stH264Avbr.u32StatTime = 0;						 // 设置统计时间
		stAttr.stRcAttr.stH264Avbr.u32Gop = gop;						 // 设置 GOP 大小
		stAttr.stRcAttr.stH264Avbr.u32SrcFrameRateNum = fps;			 // 设置源帧率
		stAttr.stRcAttr.stH264Avbr.u32SrcFrameRateDen = 1;				 // 设置源帧率分母
		stAttr.stRcAttr.stH264Avbr.fr32DstFrameRateNum = fps;			 // 设置目标帧率
		stAttr.stRcAttr.stH264Avbr.fr32DstFrameRateDen = 1;				 // 设置目标帧率分母
	}
	else if (enType == RK_VIDEO_ID_HEVC) // 如果编码类型为 H.265
	{
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265AVBR;				 // 设置为 H.265 自适应比特率模式
		stAttr.stRcAttr.stH265Avbr.u32BitRate = bitrate * 1024;			 // 设置比特率
		stAttr.stRcAttr.stH265Avbr.u32MaxBitRate = (bitrate + 1) * 1024; // 设置最大比特率
		stAttr.stRcAttr.stH265Avbr.u32MinBitRate = 0;					 // 设置最小比特率为 0
		stAttr.stRcAttr.stH265Avbr.u32StatTime = 0;						 // 设置统计时间
		stAttr.stRcAttr.stH265Avbr.u32Gop = gop;						 // 设置 GOP 大小
		stAttr.stRcAttr.stH265Avbr.u32SrcFrameRateNum = fps;			 // 设置源帧率
		stAttr.stRcAttr.stH265Avbr.u32SrcFrameRateDen = 1;				 // 设置源帧率分母
		stAttr.stRcAttr.stH265Avbr.fr32DstFrameRateNum = fps;			 // 设置目标帧率
		stAttr.stRcAttr.stH265Avbr.fr32DstFrameRateDen = 1;				 // 设置目标帧率分母
	}
	else if (enType == RK_VIDEO_ID_MJPEG) // 如果编码类型为 MJPEG
	{
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;		// 设置为 MJPEG CBR 模式
		stAttr.stRcAttr.stMjpegCbr.u32BitRate = bitrate * 1024; // 设置比特率
	}

	// 设置编码通道的其他属性
	stAttr.stVencAttr.enType = enType;				   // 设置编码类型
	stAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP; // 设置像素格式为 YUV420SP
	if (enType == RK_VIDEO_ID_AVC)
		stAttr.stVencAttr.u32Profile = H264E_PROFILE_MAIN; // 设置 H.264 的信号质量
	else if (enType == RK_VIDEO_ID_HEVC)
		stAttr.stVencAttr.u32Profile = H265E_PROFILE_MAIN; // 设置 H.265 的信号质量

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

	VENC_RECV_PIC_PARAM_S stRecvParam;						// 定义接收参数结构体
	memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S)); // 清零接收参数结构体

	stRecvParam.s32RecvPicNum = -1;					 // 设置接收图片数量为-1（表示不限制）
	RK_MPI_VENC_StartRecvFrame(chnId, &stRecvParam); // 开始接收编码帧

	return 0; // 返回成功
}
