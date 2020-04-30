/*************************************************************************
	> File Name: common_n.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年06月21日 星期五 19时07分04秒
 ************************************************************************/
#include "common_n.h"

int socket_create(int port) {
    int socketfd;
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    } 
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return -1;
    }
    listen(socketfd, 20);
    return socketfd;
}

int socket_connect(int port, char *host) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
    if (connect(socketfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return -1;
    }
    return socketfd;
}


