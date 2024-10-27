/*****************************************************************************
* | Author      :   Jason
* | Function    :   
* | Info        :
*
*----------------
* | This version:   V1.0
* | Date        :   2024-10-02
* | Info        :   Basic version
*
******************************************************************************/

#include "vtx_socket.h"

#define DEST_PORT 1234   
#define DSET_IP_ADDRESS  "192.168.0.100"   

static pthread_t g_SocketThd;

static bool g_StopSocketThread = false;
static bool new_data = false;

static uint8_t *g_frame;
static int g_len;
static uint64_t g_ts;

static void *SocketThread(void *arg)
{
    int SocketFd;
    struct sockaddr_in udpaddr;
    socklen_t AddrLen;
    int SendLen;

    // 第1步：创建socket。
    SocketFd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (SocketFd < 0)
    {
        printf("create socket error [%d].\n", errno);
        return NULL;
    }

    udpaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET。
    udpaddr.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS); // 指定目标ip地址。
    udpaddr.sin_port = htons(DEST_PORT);  // 指定目标通信端口，需要网络序转换
    AddrLen = sizeof(udpaddr);

    while (!g_StopSocketThread)
    {
        if(new_data)
        {
            printf("g_len:%d\n",g_len);
            SendLen = sendto(SocketFd, g_frame, 2000, 0, (struct sockaddr *)&udpaddr, AddrLen);
            printf("SendLen:%d\n",SendLen);

            new_data = false;

            if (SendLen <= 0)
            {
                printf("SendLen <= 0\n");
                continue;
            }
        }
    }

    close(SocketFd);
    return NULL;
}


int vi_vtx_socket_init(void)
{
	g_StopSocketThread = false;
    pthread_create(&g_SocketThd, NULL, SocketThread, NULL);  //创建TsTthread多线程互斥锁

    return 0;
}

int vi_vtx_socket_deinit(void)
{
    g_StopSocketThread = true;
    pthread_join(g_SocketThd, NULL);

    return 0;
}

int vtx_socket_get_data(uint8_t *frame, int len, uint64_t ts)
{
    if(!new_data)
    {
        g_frame = frame;
        g_len = len;
        g_ts = ts;

        new_data = true;
    }

    return 0;
}