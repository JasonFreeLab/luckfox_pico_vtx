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

#ifndef __VTX_SOCKET_H
#define __VTX_SOCKET_H

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
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sample_comm.h"

int vi_vtx_socket_init(void);
int vi_vtx_socket_deinit(void);
int vtx_socket_get_data(uint8_t *frame, int len, uint64_t ts);

#endif