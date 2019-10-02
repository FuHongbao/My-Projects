/*************************************************************************
	> File Name: util.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月28日 星期六 11时39分15秒
 ************************************************************************/

#include <stdio.h>
#include "util.h"

int socket_create(int port) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return 1;
    }
    int flag = 1, len = sizeof(flag);
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, len) < 0) {
        close (fd);
        perror("setsockopt");
        return 1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error bind : fd");
        return 1;
    }
    listen(fd, 200);
    return fd;
}

int socket_connect(int port, char *ip) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error socket : fd");
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return 1;
    }
    return fd;
}

