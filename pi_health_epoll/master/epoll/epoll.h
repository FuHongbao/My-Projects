/*************************************************************************
	> File Name: epoll.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月26日 星期日 11时19分54秒
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H

#include "../node/node.h"
#include "../common/common.h"
#include "../pthread_pool/threadpool.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAXEVENTS 100
#define FILE_N  6
#define MAXBUFFSIZE 1024
typedef struct Epoll_Param {
    HashFind *table;
    int epollfd;
    int sockfd;
} Epoll_Param;

typedef struct User_Msg {
    int fd;
    struct sockaddr_in addr;
    char buff[MAXBUFFSIZE];
} User_Msg;

int epollfd;
User_Msg user_msg[1000];
HashFind *table;
Conf conf;
Task_Object task_obj[FILE_N];
#define error_log "./error.log"
#define base_log "./log"


void do_epoll(HashFind *table, int listenfd, int datafd);
void handle_events(HashFind *table, int epollfd, struct epoll_event *events, int num, int listenfd, int datafd);
void add_event(int epollfd, int sockfd, int state) ;
void delete_event(int epollfd, int sockfd, int state);
void modify_event(int epollfd, int sockfd, int state);

void do_data(int fd, char *ip);
void *Handle_Accept(void *arg);
void *Handle_Read(void *arg);
void *Handle_Write(void *arg);
void *handle_Data(void *arg);
#endif
