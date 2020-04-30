/*************************************************************************
	> File Name: common_n.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年06月21日 星期五 19时06分13秒
 ************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#ifndef _COMMON_N_H
#define _COMMON_N_H
int socket_create(int port);
int socket_connect(int port, char *host);

#endif
