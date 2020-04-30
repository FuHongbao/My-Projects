/*************************************************************************
	> File Name: client.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月28日 星期二 21时48分29秒
 ************************************************************************/

#ifndef _CLIENT_H
#define _CLIENT_H
#include "./common/common.h"
#define FILE_N 6
#define Error_Log "./error.log"
#define BUFFSIZE 1024
#define MAXSLEEPTIME 100
#define DATATRANSTIME 20
#define LOGPATH "./log"

Conf conf;
Task_Object task_obj[FILE_N];
pthread_mutex_t smutex;
pthread_cond_t scond;
double dyAver = 0;
char tmp_info[6][BUFFSIZE * 4] = {0};
int connect_flag = 0;

void *do_heart(void *arg);
void *do_data(void *arg);
void *do_bash(void *arg);
void send_warn(char *warn_data);

void *do_heart(void *arg) {
    int sleep_time = 10;
    while (true) {
    connect_flag = 0;
    printf("try to connect master\n");
    int sockfd = socket_connect_tcp(conf.master_port, conf.ip);
    if (sockfd < 0) {
        write_log(Error_Log, "[心跳模块] [error] [message : failed to connect master]");
        sleep(sleep_time);
        continue;
    }
    printf("connect success\n");
    char buff[BUFFSIZE];
    while (true) {
        strcpy(buff, "heart");
        int ret = send(sockfd, buff, strlen(buff), 0);
        if (ret <= 0) {
            write_log(Error_Log, "[心跳模块] [error] [message : %s]", strerror(errno));
            break;
        }
        memset(buff, 0, sizeof(buff));
        fd_set rfds;
        struct timeval timeout = {conf.time_sec, conf.time_usec};
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        ret = select(sockfd + 1, &rfds, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("select");
            break;
        }
        if (0 == ret) {
            break;
        } else {
            if (FD_ISSET(sockfd, &rfds)) {
                ret = recv(sockfd, buff, sizeof(buff), 0);
                if (ret <= 0) {
                    write_log(Error_Log, "[心跳模块] [error] [message : %s]", strerror(errno));
                    break;
                }
                printf("heart check : recv[%s]\n", buff); 
                connect_flag = 1;
                sleep(5);
            }
        }
    }
    close(sockfd);
    sleep(sleep_time);
    if (sleep_time < 100) sleep_time += 10;
    printf("retry connect\n");
    }

}

void *do_data(void *arg) {
    while (true) {
        sleep(DATATRANSTIME);
        if (!connect_flag) continue;
        int sockfd = socket_connect_tcp(conf.ctrl_port, conf.ip);
        if (sockfd < 0) {
            write_log("[数据模块] [error] [message : %s]", strerror(errno));
            sleep(MAXSLEEPTIME);
            continue;
        }
        int task_num = 0;
        for (int i = 0; i < 6; i++) {
            int ret = recv(sockfd, &task_num, sizeof(int), 0);
            if (ret < 0) {
                perror("recv");
                write_log("[数据模块] [error] [message : %s]", strerror(errno));
                int ack = 0;
                send(sockfd, &ack, sizeof(int), 0);
                continue;
            }
            int ack = 0;
            int datafd;
            char path[100] = {0};
            sprintf(path, "%s/%s", LOGPATH, task_obj[i].filename);
            if (task_num == task_obj[i].Req_num) {
                if (access(path, F_OK) >= 0) {
                    ack = task_obj[i].Ack_num;
                    datafd = socket_create_tcp(conf.data_port);
                }
            }
            ret = send(sockfd, &ack, sizeof(int), 0);
            if (ack == 0) continue;
            if (ret <= 0) {
                perror("send");
                write_log(Error_Log, "[数据模块] [error] [message : %s]", strerror(errno));
                close(datafd); 
                continue;
            }
            struct sockaddr_in taddr;
            socklen_t len = sizeof(taddr);
            int transfd = accept(datafd, (struct sockaddr*)&taddr, &len);
            if (transfd < 0) {
                perror("accept");
                write_log(Error_Log, "[数据模块] [error] [message : %s]", strerror(errno));
                close(datafd);
                continue;
            }
            FILE *fp = fopen(path, "r");
            if (fp == NULL) {
                printf("failed to open log[%d]", i);
                exit(1);
            }
            flock(fp->_fileno, LOCK_EX);
            char buff[BUFFSIZE] = {0};
            while(fgets(buff, BUFFSIZE, fp) != NULL) {
                send(transfd, buff, strlen(buff), 0);
                memset(buff, 0, sizeof(buff));
            }
            fclose(fp);
            close(transfd);
            close(datafd);
            remove(path);
        }
        close(sockfd);
    }
}

void send_warn(char *warn_data) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        write_log(Error_Log, "[警报模块] [error] [message : %s]", strerror(errno));
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(conf.warn_port);
    addr.sin_addr.s_addr = inet_addr(conf.ip);
    sendto(sockfd, warn_data, strlen(warn_data), 0, (struct sockaddr*)&addr, sizeof(addr));
    printf("send warn[%s], ip[%s], port[%d]\n", warn_data, conf.ip, conf.warn_port);
    close(sockfd); 
}


void *do_bash(void *arg) {
    int *temp = (int*)arg;
    int inx = *temp;
    char *script_path = "./script";
    int cnt = 0;
    while (true) {
        char opstr[100] = {0};
        FILE *pfile = NULL;
        if (inx == 3) {
            sprintf(opstr, "bash %s/%s %lf", script_path, task_obj[inx].shellname, dyAver);
        } else {
            sprintf(opstr, "bash %s/%s", script_path, task_obj[inx].shellname);
        }
        pfile = popen(opstr, "r");
        if (!pfile) {
            write_log(Error_Log, "[脚本执行] [error] [message : %s]", strerror(errno));
            sleep(5);
            continue;
        }
        char buff[BUFFSIZE] = {0};
        if (inx == 3) {
            if (fgets(buff, BUFFSIZE, pfile) != NULL) {
                strcat(tmp_info[inx], buff);
            }
            if (fgets(buff, BUFFSIZE, pfile) != NULL) {
                dyAver = atof(buff);
            }
        } else {
            while (fgets(buff, BUFFSIZE, pfile) != NULL) {
                strcat(tmp_info[inx], buff);
            }
        }
        if ((inx == 0 && strstr(buff, "warning")) || (inx == 2 && strcmp(buff, ""))) {
            send_warn(buff); 
        }

        pclose(pfile);
        cnt++;
        if (cnt == 5) {
            char dest[100] = {0};
            sprintf(dest, "%s/%s", LOGPATH, task_obj[inx].filename);
            FILE *fw = fopen(dest, "a+");
            if (!fw) {
                write_log(Error_Log, "[脚本执行] [error] [message : %s]", strerror(errno));
                sleep(5);
                continue;
            }
            flock(fw->_fileno, LOCK_EX);
            fprintf(fw, "%s", tmp_info[inx]);
            fclose(fw);
            memset(tmp_info[inx], 0, sizeof(tmp_info[inx]));
            cnt = 0;
            printf("bash once [%d]\n", inx);
        }
        sleep(5);
    }
}

#endif
