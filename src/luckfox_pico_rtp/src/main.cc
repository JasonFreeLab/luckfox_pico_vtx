/*****************************************************************************
* | Author      :   Luckfox team
* | Function    :   
* | Info        :
*
*----------------
* | This version:   V1.0
* | Date        :   2024-04-07
* | Info        :   Basic version
*
******************************************************************************/

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

// #include "rtsp_demo.h"
#include "luckfox_mpi.h"

#include "gst_push.h"

// #define HOST_IP "127.0.0.1"
// #define HOST_PORT 5000

#define VIDEO_WIGHT 1280
#define VIDEO_HEIGHT 720
// #define VIDEO_FPS 60

int main(int argc, char *argv[])
{
	system("RkLunch-stop.sh");

	// rkaiq init
	RK_BOOL multi_sensor = RK_FALSE;
	const char *iq_dir = "/etc/iqfiles";
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
	SAMPLE_COMM_ISP_Init(0, hdr_mode, multi_sensor, iq_dir);
	SAMPLE_COMM_ISP_Run(0);

	// rkmpi init
	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		RK_LOGE("rk mpi sys init fail!");
		return -1;
	}

	// rtsp init	
	// rtsp_demo_handle g_rtsplive = NULL;
	// rtsp_session_handle g_rtsp_session;
	// g_rtsplive = create_rtsp_demo(554);
	// g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/0");
	// rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H265, NULL, 0);
	// rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime());

	// rtp init
	// InputParameters rtp_params;
	// rtp_params = {
    //     .g_host = HOST_IP, 		// 目标主机
    //     .g_port = HOST_PORT,        		// 目标端口
    //     .width = VIDEO_WIGHT,       // 视频宽度
    //     .height = VIDEO_HEIGHT,     // 视频高度
    //     .fps = VIDEO_FPS,           // 帧率
    //     .codec = "video/x-h265",    // 编码格式
    //     .type = "rtph265pay",       // 编码器类型
    //     .frameData = NULL,          // 初始化为NULL，稍后分配
    //     .frameSize = 0,				// 设置帧大小，初始化为0，稍后设置
    //     .pts = 0,                   // 初始化PTS，初始化为0，稍后设置
    //     .framesSent = 0,            // 已发送的帧计数，初始化为0
    //     .gst_element = NULL         // 初始化gst_element为NULL
    // };
	// gstreamer_push_init(&rtp_params); // 初始化 GStreamer 流水线
	main_init();

	// vi init
	vi_dev_init();
	vi_chn_init(0, VIDEO_WIGHT, VIDEO_HEIGHT);

	// venc init
	RK_CODEC_ID_E enCodecType = RK_VIDEO_ID_HEVC;
	venc_init(0, VIDEO_WIGHT, VIDEO_HEIGHT, enCodecType);
	
	// bind vi to venc
	MPP_CHN_S stSrcChn, stvencChn;

	stSrcChn.enModId = RK_ID_VI;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;

	stvencChn.enModId = RK_ID_VENC;
	stvencChn.s32DevId = 0;
	stvencChn.s32ChnId = 0;
	printf("====RK_MPI_SYS_Bind vi0 to venc0====\n");
	RK_S32 s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stvencChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("bind 0 ch venc failed");
		return -1;
	}

	float fps = 0;
	guint64 frame_duration = 1000000 / 30; // 计算每帧间隔（微秒）

	//h265_frame	
	VENC_STREAM_S stFrame;	
	stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

	// void *pData = RK_NULL;

	while(1)
	{
		// rtsp
		s32Ret = RK_MPI_VENC_GetStream(0, &stFrame, -1);
		if(s32Ret == RK_SUCCESS)
		{
			// if(g_rtsplive && g_rtsp_session)
			// {
			// 	printf("fps = %.2f\n",fps);
			// 	//printf("len = %d PTS = %d \n",stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS);	
			// 	pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
			// 	rtsp_tx_video(g_rtsp_session, (uint8_t *)pData, stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS);
			// 	rtsp_do_event(g_rtsplive);
			// }
			// RK_U64 nowUs = TEST_COMM_GetNowUs();
			// fps = (float) 1000000 / (float)(nowUs - stFrame.pstPack->u64PTS);

			// printf("fps = %.2f\n",fps);

			// rtp_params.frameData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
			// rtp_params.frameSize = stFrame.pstPack->u32Len;
			// rtp_params.pts = stFrame.pstPack->u64PTS;

			// if (!gstreamer_push_frame_data(&rtp_params)) {
            // 	printf("推送帧数据失败");
        	// }

			// RK_U64 nowUs = TEST_COMM_GetNowUs();
			// fps = (float) 1000000 / (float)(nowUs - rtp_params.pts);

			g_usleep(frame_duration); // 控制帧率
		}

		// release frame 
		s32Ret = RK_MPI_VENC_ReleaseStream(0, &stFrame);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
		}

	}
	// printf("已发送帧数: %d\n", rtp_params.framesSent); // 输出已发送的帧数
	// gstreamer_push_deinit(&rtp_params); // 去初始化 GStreamer 会话

	RK_MPI_SYS_UnBind(&stSrcChn, &stvencChn);
	
	RK_MPI_VI_DisableChn(0, 0);
	RK_MPI_VI_DisableDev(0);
	
	SAMPLE_COMM_ISP_Stop(0);

	RK_MPI_VENC_StopRecvFrame(0);
	RK_MPI_VENC_DestroyChn(0);

	free(stFrame.pstPack);

	// if (g_rtsplive)
	// 	rtsp_del_demo(g_rtsplive);
	
	RK_MPI_SYS_Exit();

	return 0;
}
