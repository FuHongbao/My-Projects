/*************************************************************************
	> File Name: client.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年08月02日 星期五 16时46分10秒
 ************************************************************************/
#include "util.h"
#define BUFFSIZE 2048

void recv_heart(int port);
void recv_data(int port);
void do_load(char *ip, int loadPort);

void do_load(char *ip, int loadPort) {
    printf("\nclient heart start\n");
    while (1) {
        int sockfd = socket_connect(loadPort, ip);
        if (sockfd < 0) {
            close(sockfd);
            printf("\ntry once\n");
            sleep(5);
            continue;
        }
        printf("\nconnect success\n");
        close(sockfd);
        break;
    }
}

void recv_data(int port) {
    int listenfd;
    if ((listenfd = socket_create(port)) < 0) {
        perror("Error socket_create on listenfd");
        return;
    }
    while (1) {
        int newfd = accept(listenfd, NULL, NULL);
        if (newfd < 0) {
            perror("accept");
            continue;
        }
        char buff[BUFFSIZE] = {0};
        int k = recv(newfd, buff, sizeof(buff), 0);
        if (k <= 0) {
            perror("recv");
            close(newfd);
            continue;
        }
        printf("\n%s\n", buff);
        memset(buff, 0, sizeof(buff));
        strcpy(buff, "bye");
        send(newfd, buff, strlen(buff), 0);
        close(newfd);
    }
    close(listenfd);
}


void recv_heart(int port) {
    int sockfd;
    sockfd = socket_create(port);
    if (sockfd < 0) {
        perror("Error bind on heartPort");
        return ;
    }
    while (1) {
        int newfd = accept(sockfd, NULL, NULL);
        printf(" ❤ ");
        fflush(stdout);
        close(newfd);
    }
    close(sockfd);
}




