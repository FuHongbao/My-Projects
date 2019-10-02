/*************************************************************************
	> File Name: ball_client.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月29日 星期日 11时47分49秒
 ************************************************************************/

#include <stdio.h>
#include "ball_client.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        perror("Error parameter");
        exit(1);
    }
    int fd;
    int port = atoi(argv[2]);
    if ((fd = socket_connect(port, argv[1])) < 0) {
        perror("Error connect : fd");
        exit(1);
    }
    myteam = atoi(argv[3]);
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    struct Message mes;
    mes.team = myteam;
    strcpy(mes.name, pwd->pw_name);
    if (send(fd, &mes, sizeof(mes), 0) < 0) {
        perror("Error send : fd");
        exit(1);
    }
    pthread_t pth_recv;
    pthread_create(&pth_recv, NULL, to_recv, &fd);
    to_send(fd);
    return 0;
}

