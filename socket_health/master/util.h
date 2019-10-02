/*************************************************************************
	> File Name: util.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年08月02日 星期五 10时36分35秒
 ************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <linux/limits.h>


#ifndef _UTIL_H
#define _UTIL_H
int get_conf_value(char *file, char *key, char *value);
int socket_create(int port);
int socket_connect(int port, char *ip);
int write_log(char *path, const char *format, ...);

#endif
