#ifndef __LUCKFOX_MPI_H
#define __LUCKFOX_MPI_H

#include "sample_comm.h"

/**
 * @brief 获取当前时间（微秒）
 *
 * @return RK_U64 返回当前时间，单位为微秒
 */
RK_U64 TEST_COMM_GetNowUs(void);

/**
 * @brief 初始化视频设备
 *
 * @return int 返回0表示成功，-1表示失败
 */
int vi_dev_init(void);

/**
 * @brief 初始化视频通道
 *
 * @param channelId 通道 ID，类型为 int
 * @param width 通道图像宽度，类型为 int
 * @param height 通道图像高度，类型为 int
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int vi_chn_init(int channelId, int width, int height);

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
int venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType);

#endif