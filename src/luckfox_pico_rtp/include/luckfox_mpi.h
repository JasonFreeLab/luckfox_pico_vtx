#ifndef __LUCKFOX_MPI_H
#define __LUCKFOX_MPI_H

#include "sample_comm.h"

/**
 * @brief 初始化视频设备
 *
 * @return int 返回0表示成功，-1表示失败
 */
int vi_dev_init(void);

/**
 * @brief 初始化视频通道
 *
 * @param channelId 通道 ID，类型为 uint8_t
 * @param width 通道图像宽度，类型为 uint16_t
 * @param height 通道图像高度，类型为 uint16_t
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int vi_chn_init(uint8_t channelId, uint16_t width, uint16_t height);

/**
 * @brief 初始化视频编码通道
 *
 * @param chnId 编码通道 ID，类型为 uint8_t
 * @param width 编码图像宽度，类型为 uint16_t
 * @param height 编码图像高度，类型为 uint16_t
 * @param enType 编码类型，类型为 RK_CODEC_ID_E
 * @param bitrate 编码比特率，类型为 uint8_t
 * @param fps 编码帧率，类型为 uint8_t
 *
 * @return int 返回0表示成功，其他值表示错误码
 */
int venc_init(uint8_t chnId, uint16_t width, uint16_t height, RK_CODEC_ID_E enType, uint8_t bitrate, uint8_t fps);

#endif