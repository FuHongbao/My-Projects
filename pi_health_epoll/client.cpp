/*************************************************************************
	> File Name: client.cpp
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年07月07日 星期日 16时22分37秒
 ************************************************************************/

#include "common_n.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int main() {
    int port = 8379;
    char ip[20] = "192.168.1.8";
    int socket = socket_connect(port, ip);
    char buff[100];
    int k=1 ;
    int num = 1;
    while (true) {

    strcpy(buff, "heart");
    if (send(socket, buff, strlen(buff), 0) < 0) {
        perror("send");
    }
    memset(buff, 0, sizeof(buff));
    if (recv(socket, buff,1024, 0) < 0) {
        perror("recv");
    }
    cout << buff << endl;
    sleep(5);
    }
    close(socket);
    return 0;
}




