/*************************************************************************
	> File Name: ball_server.cpp
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月28日 星期六 11时07分01秒
 ************************************************************************/

#include <stdio.h>
#include "ball_server.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        perror("Error parameter");
        exit(1);
    }
    struct Message mes;    
    int port = atoi(argv[1]);
    int listen_fd;
    if ((listen_fd = socket_create(port)) == 0) {
        perror("Error socket_create : listen_fd");
        exit(1);
    }
    memset(&game, 0, sizeof(game));
    pthread_t pth_send;
    pthread_create(&pth_send, NULL, to_send, NULL);
    while (1) {
        int new_fd;
        if ((new_fd = accept(listen_fd, NULL, NULL)) < 0) {
            perror("Error accept : new_fd");
            exit(1);
        }
        memset(&mes, 0, sizeof(mes));
        if (recv(new_fd, &mes, sizeof(mes), 0) < 0) {
            perror("Error recv : new_fd");
            close(new_fd);
            close(listen_fd);
            exit(1);
        }
        srand((unsigned)time(0) * 10);
        game.peo[game.num].loc_x = rand() % 32 + 1;
        game.peo[game.num].team = mes.team;
        switch(mes.team) {
            case 1: {
                game.peo[game.num].loc_y = rand() % 54 + 1;
            } break;
            case 2: {
                game.peo[game.num].loc_y = rand() % 54 + 56;
            }break;
            default :
            break;
        }
        game.peo[game.num].fd = new_fd;
        mes.pno = game.num;
        game.num += 1;
        FILE *fp;
        fp = fopen("./game.log", "ab+");
        fprintf(fp, "%s login : online num -- %d\n", mes.name, game.num);
        fclose(fp);
        mes.fd = new_fd;
        pthread_t pth_recv;
        pthread_create(&pth_recv, NULL, to_recv, &mes);
    }
    close (listen_fd);
    return 0;
}


