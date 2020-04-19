/*************************************************************************
	> File Name: master.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年03月29日 星期日 16时55分03秒
 ************************************************************************/

#include "util/util.h"
#include <pthread.h>
#define MAXCLIENTNUM 10000
#define BUFFSIZE     2048

typedef struct Node {
    struct sockaddr_in addr;
    int fd;
    struct Node *next;
} Node, *LNode;

struct PthHeartCheck {
    int *sum;
    LNode *list;
    int ins;
    long timeout;
};

struct PthDataTrans {
    LNode head;
    int ind;
    int ctlPort;
    int dataPort;
};

void output(LNode head);

int insert(LNode head, LNode node);
int find_min(int *sum, int ins);
int check_connect(struct sockaddr_in addr, long timeout);
int listen_client(int listenfd, LNode *list, int *sum, int ins, int heartPort);
void *do_heart(void *arg);
void *do_data(void *arg);
void *do_warn(void *arg);
void output(LNode head);

char *mpath = "/opt/pi_master/log/";
char *warnMessage = "/opt/pi_master/warn.log";
char *errorLogPath = "/opt/pi_master/error.log";


int find_min(int *sum, int ins) {
    int min = 999999, ind;
    for(int i = 0; i < ins; i++) {
        if (sum[i] < min) {
            min = sum[i];
            ind = i;
        }
    }
    return ind;
}

int insert(LNode head, LNode node) {
    Node *p = head;
    while(p->next) {
        p = p->next;
    }
    p->next = node;
    return 0;
}

void output(LNode head) {
    LNode p = head;
    int cnt = 0;
    while(p->next) {
        printf("[%d] : %s\n", ++cnt, inet_ntoa(p->next->addr.sin_addr));
        p = p->next;
    }
    return ;
}

void *do_heart(void *arg) {
    struct PthHeartCheck *harg = (struct PthHeartCheck*)arg;
    while(1) {
        //printf("start do heart\n");
        for (int i = 0; i < harg->ins; i++) {
            Node *p = harg->list[i];
            while (p->next) {
                if (check_connect(p->next->addr, harg->timeout) < 0) {
                    //printf("%s is \033[31mnot online\033[0m on %d\n", inet_ntoa(p->next->addr.sin_addr), ntohs(p->next->addr.sin_port));
                    Node *q = p->next;
                    p->next = q->next;
                    free(q);
                    harg->sum[i] -= 1;
                } else {
                    //printf("%s is \033[32monline\033[0m on %d\n", inet_ntoa(p->next->addr.sin_addr), ntohs(p->next->addr.sin_port));
                    p = p->next;
                }
            }
        }
        sleep(5);
        printf("\n");
    }
}

int check_connect(struct sockaddr_in addr, long timeout) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        write_log(errorLogPath, "[心跳模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return -1;
    }
    unsigned long ul = 1;
    ioctl(sockfd, FIONBIO, &ul);
    struct timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = timeout;
    fd_set wset;
    FD_ZERO(&wset);
    FD_SET(sockfd, &wset);
    int error = -1, len = sizeof(int), ret = -1;
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if (select(sockfd + 1, NULL, &wset, NULL, &tm) > 0) {
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len);
            if (error == 0) {
                ret = 0;
            } else {
                ret = -1;
            }
        } else {
            ret = -1;
        }
    }
    close(sockfd);
    return ret;
}

void *do_data(void *arg) {
    struct PthDataTrans *darg = (struct PthDataTrans*)arg;
    char filename[6][20] = {"cpu.log", "disk.log", "mem.log", "user.log", "sys.log", "enermy.log"};
    while (1) {
        sleep(15);
        Node *p = darg->head;
        while (p->next) {
           // printf("start trans --- ");
            printf("%s\n", inet_ntoa(p->next->addr.sin_addr));
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                p = p->next;
                continue;
            }
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(darg->ctlPort);
            addr.sin_addr = p->next->addr.sin_addr;
            if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                close(sockfd);
                write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                p = p->next;
                continue;
            }
            for (int i = 100; i <= 105; i++) {
                int ret = send(sockfd, &i, sizeof(int), 0);
                if (ret < 0) {
                    write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    continue;
                }
                int ack = 0;
                int rek = recv(sockfd, &ack, sizeof(int), 0);
                if (rek <= 0) {
                    write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    continue;
                }
                if (!ack) continue;
                int recvfd = socket_connect(darg->dataPort, inet_ntoa(p->next->addr.sin_addr));

                if (recvfd < 0) {
                    write_log(errorLogPath, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    continue;
                }
                char fdir[100] = {0};
                sprintf(fdir, "%s/%s", mpath, inet_ntoa(p->next->addr.sin_addr));
                if (access(fdir, F_OK) < 0) {
                    int status = mkdir(fdir, 0755);
                    if (status < 0) {
                        close(recvfd);
                        continue;
                    }
                }
                char filepath[100] = {0};
                sprintf(filepath, "%s/%s", fdir, filename[i - 100]);
                char buff[BUFFSIZE] = {0};
                FILE *fw = fopen(filepath, "a+");
                while (1) {
                    int k = recv(recvfd, buff, sizeof(buff), 0);
                    if (k <= 0) break;
                    fprintf(fw, "%s", buff);
                    memset(buff, 0, sizeof(buff));
                }
                fclose(fw);
                close(recvfd);
            }
            close(sockfd);
            p = p->next;
        }
    }
}


void *do_warn(void *arg) {
    //printf("start do warn\n");
    int *inarg = (int*) arg;
    int port = *inarg;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        write_log(errorLogPath, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return NULL;
    }
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        write_log(errorLogPath, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        close(sockfd);
        return NULL;
    }
    struct sockaddr_in addr, claddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        write_log(errorLogPath, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        close(sockfd);
        return NULL;
    }
    socklen_t len = sizeof(claddr);
    char buff[BUFFSIZE] = {0};
    while (1) {
        memset(buff, 0, sizeof(buff));
        recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&claddr, &len);
        FILE *fp = fopen(warnMessage, "a+");
        fprintf(fp, "%s", buff);
        fclose(fp);
    }
    close(sockfd);
}


int listen_client(int listenfd, LNode *list, int *sum, int ins, int heartPort) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    while (1) {
        //printf("start do listen\n");
        int newfd = accept(listenfd, (struct sockaddr*)&addr, &len);
        if (newfd < 0) {
            write_log(errorLogPath, "[监听模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
            return -1;
        }
        int ind = find_min(sum, ins);
        Node *p = (Node *) malloc(sizeof(Node));
        addr.sin_port = htons(heartPort);
        p->fd = newfd;
        p->addr = addr;
        p->next = NULL;
        insert(list[ind], p);
        sum[ind] += 1;
        printf("someone come in\n");
        close(newfd);
    }
    close(listenfd);
}

