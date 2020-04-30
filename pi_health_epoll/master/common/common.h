/*************************************************************************
	> File Name: common.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月25日 星期六 18时29分47秒
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <linux/limits.h>

typedef struct Task_Object {
    int Req_num;
    int Ack_num;
    char *filename;
} Task_Object;

typedef struct Master_Conf {
    char *Start_Ip;
    char *End_Ip;
    char *log;
    int listen_port;
    int trans_port;
    int ctrl_port;
    int data_port;
    int warn_port;
    int INS;
} Conf;


char *load_config(char *file, char *key);
int write_log(char *path, const char *format, ...);
int socket_create_tcp(int port);
int socket_connect_tcp(int port, char *ip);
int socket_create_udp(int port);

#endif
