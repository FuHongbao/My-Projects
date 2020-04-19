/*************************************************************************
	> File Name: client.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年03月31日 星期二 20时02分11秒
 ************************************************************************/

#include "./util/util.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <termios.h>
#include <pthread.h>
#define BUFFSIZE 2048

pthread_mutex_t smutex;
pthread_cond_t scond;
int shareCnt = 0;


struct PthDataTrans {
    int dataPort;
    int ctlPort;
};

struct PthHeartCheck {
    int heartPort;
};

struct PthHealthCheck {
    int inx;
    int warnPort;
    char *ip;
};

char shellname[6][20] = {"cpu_info.sh", "disk_info.sh", "mem_info.sh", "user_info.sh", "SysInfo.sh", "enermy_pro.sh"};
char logname[6][20] = {"cpu.log", "disk.log", "mem.log", "user.log", "sys.log", "enermy.log"};
char tmp_info[BUFFSIZE * 4] = {0};
double dyAver = 0;
char *script_path = "/opt/pi_client/script";
char *log_path = "/opt/pi_client/log";
char *errorLogPath = "/opt/pi_client/error.log";

int is_use(int port);
void *do_data(void *arg);
void *do_heart(void *arg);
void *do_check(void *arg);
void do_load(char *ip, int port);
int do_bash(int inx, int cnt, int port, char *ip);
void send_warn(char *ip, int port, char *message, int ind);


int is_use(int port) {
    int sockfd = socket_listen(port);
    int ret = 0;
    if (sockfd < 0) {
        ret = -1;
    } else {
        close(sockfd);
    }
    return ret;
}

void *do_heart(void *arg) {
    struct PthHeartCheck *harg = (struct PthHeartCheck*)arg;
    int sockfd;
    sockfd = socket_listen(harg->heartPort);
    if (sockfd < 0) {
        write_log(errorLogPath, "[心跳模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return NULL;
    }
    while (1) {
        int newfd = accept(sockfd, NULL, NULL);
        if (newfd < 0) {
            write_log(errorLogPath, "[心跳模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        }
        printf(" ❤ ");
        fflush(stdout);
        pthread_mutex_lock(&smutex);
        shareCnt = 0;
        pthread_mutex_unlock(&smutex);
        close(newfd);
    } 
    close(sockfd);
}

void *do_check(void *arg) {
    struct PthHealthCheck *carg = (struct PthHealthCheck*)arg;
    int cnt = 0;
    while (1) {
        cnt ++;
        do_bash(carg->inx, cnt, carg->warnPort, carg->ip);
        if (carg->inx == 0) {
            pthread_mutex_lock(&smutex);
            if (shareCnt == 5) {
                pthread_mutex_unlock(&smutex);
                return NULL;
            }
            if (++shareCnt >= 5) {
                pthread_cond_signal(&scond);
            }
            pthread_mutex_unlock(&smutex);
        }
        sleep(2);
        if (cnt == 5) cnt = 0;
    }
}

int do_bash(int inx, int cnt, int port, char *ip) {
    char opstr[100] = {0};
    FILE *pfile = NULL;
    if (inx == 2) {
        sprintf(opstr, "bash %s/%s %lf", script_path, shellname[inx], dyAver);
    } else {
        sprintf(opstr, "bash %s/%s", script_path, shellname[inx]);
    }
    pfile = popen(opstr, "r");
    if (!pfile) {
        write_log(errorLogPath, "[脚本执行] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return -1;
    }
    char buff[BUFFSIZE] = {0};
    if (inx == 2) {
        if (fgets(buff, BUFFSIZE, pfile) != NULL) {
            strcat(tmp_info, buff);
        }
        if (fgets(buff, BUFFSIZE, pfile) != NULL) {
            dyAver = atof(buff);
        }
    } else {
        while (fgets(buff, BUFFSIZE, pfile) != NULL) {
            strcat(tmp_info, buff);
        }
    }
    pclose(pfile);
    if (shareCnt < 5 && (inx == 5 || inx == 0)) {
        char warnbuff[BUFFSIZE] = {0};
        strcpy(warnbuff, buff);
        send_warn(ip, port, warnbuff, inx);
    }
    if (cnt == 5) {
        char dest[100] = {0};
        sprintf(dest, "%s/%s", log_path, logname[inx]);
        FILE *fw = fopen(dest, "a+");
        if (!fw) {
            write_log(errorLogPath, "[脚本执行] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
            return -1;
        }
        flock(fw->_fileno, LOCK_EX);
        fprintf(fw, "%s", tmp_info);
        fclose(fw);
        memset(tmp_info, 0, sizeof(tmp_info));
    }
    return 0;
}

void send_warn(char *ip, int port, char *message, int ind) {
    if (ind == 5 && !strcmp(message, "")) return;
    if (ind == 0 && strstr(message, "warning") == NULL) return;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        write_log(errorLogPath, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sockfd);
}

void *do_data(void *arg) {
    struct PthDataTrans *darg = (struct PthDataTrans*)arg;
    int listenfd = socket_listen(darg->ctlPort);
    if (listenfd < 0) {
        write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return NULL;
    }
    while (1) {
        int newfd = accept(listenfd, NULL, NULL);
        if (newfd < 0) {
            write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]",getpid(), strerror(errno));
            continue;
        }
        for (int i = 0; i < 6; i++) {
            int fno = -1;
            int ret = recv(newfd, &fno, sizeof(int), 0);
            if (ret <= 0) {
                write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                break;
            }
            char path[50] = {0};
            sprintf(path, "%s/%s", log_path, logname[fno - 100]);
            int ack = 0;
            if (access(path, F_OK) < 0) {
                send(newfd, &ack, sizeof(int), 0);
                continue;
            }
            int sendfd = socket_listen(darg->dataPort);
            if (sendfd < 0) {
                ack = 0;
                send(newfd, &ack, sizeof(int), 0);
                continue;
            }
            ack = 1;
            send(newfd, &ack, sizeof(int), 0);
            int nsendfd = accept(sendfd, NULL, NULL);
            if (nsendfd < 0) {
                write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                close(sendfd);
                continue;
            }
            FILE *fp = fopen(path, "r");
            flock(fp->_fileno, LOCK_EX);
            char buff[BUFFSIZE] = {0};
            while (fgets(buff, BUFFSIZE, fp) != NULL) {
                send(nsendfd, buff, strlen(buff), 0);
                memset(buff, 0, sizeof(buff));
            }
            fclose(fp);
            close(nsendfd);
            close(sendfd);
            remove(path);
        }
        close(newfd);
        pthread_mutex_lock(&smutex);
        shareCnt = 0;
        pthread_mutex_unlock(&smutex);
    }
    close(listenfd);
}

void do_load(char *ip, int port) {
    printf("\nclient try load start\n");
    int stime = 10;
    while (1) {
        pthread_mutex_lock(&smutex);
        if (shareCnt < 5) {
            pthread_mutex_unlock(&smutex);
            break;
        }
        pthread_mutex_unlock(&smutex);
        int sockfd = socket_connect(port, ip);
        if (sockfd < 0) {
            close(sockfd);
            write_log(errorLogPath, "[连接模块] [error] [process : %d] [message : failed to connect master]", getpid());
            sleep(stime);
            if (stime < 100) stime += 10;
            continue;
        }
        close(sockfd);
        break;
    }
    return;
}


