/*************************************************************************
	> File Name: master.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年08月02日 星期五 11时33分05秒
 ************************************************************************/
#include "util.h"
#include <pthread.h>
#define MAXCLIENT 10000
#define BUFFSIZE 2048

typedef struct Node {
    struct sockaddr_in addr;
    int fd;
    struct Node *next;
}Node, *LinkList;

struct Heart {
    int *sum;
    LinkList *linklist;
    int ins;
    long timeout;
} ;

struct Data {
    LinkList head;
    int ind;
    int ctlport;
    int dataport;
};


int insert (LinkList head, Node *node);
void output(LinkList head);
int find_min(int *sum, int ins) ;
void *do_heart(void *arg);
void *do_data(void *arg);
void *do_warn(void *arg);
int check_connect(struct sockaddr_in addr, long timeout);
void listen_epoll(int listenfd, LinkList *linklist, int *sum, int ins, int heartPort);


char *Error_master = "/opt/pi_master/Error_master.log";
char *mpath = "/opt/pi_master/log/";
char *warnMessage = "/opt/pi_master/Warn.log";

int find_min(int *sum, int ins) {
    int min = 999999, ind;
    for (int i = 0; i < ins; i++) {
        if (sum[i] < min) {
            min = sum[i];
            ind = i;
        }
    }
    return ind;
}

int insert(LinkList head, Node *node) {
    Node *p = head; 
    while (p->next) {
        p = p->next;
    }
    p->next = node;
    return 1;
}

void output(LinkList head) {
    Node *p = head;
    int cnt = 0;
    while (p->next) {
        printf("[%d] : %s\n", ++cnt, inet_ntoa(p->next->addr.sin_addr));
        p = p->next;
    }
}

void *do_heart(void *arg) {
    struct Heart *harg = (struct Heart*)arg;
    while (1) {
        for (int i = 0; i < harg->ins; i++) {
            Node *p = harg->linklist[i];
            while (p->next) {
                if (check_connect(p->next->addr, harg->timeout) < 0) {
                    printf("%s is \033[31mnot online\033[0m on %d\n", inet_ntoa(p->next->addr.sin_addr), ntohs(p->next->addr.sin_port));
                    Node *q = p->next;
                    p->next = q->next;
                    free(q);
                    harg->sum[i] -= 1;
                } else {
                    printf("%s is \033[32monline\033[0m on %d\n", inet_ntoa(p->next->addr.sin_addr), ntohs(p->next->addr.sin_port));
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
        perror("socket");
        write_log(Error_master, "[心跳模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
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
    int error = -1;
    int len = sizeof(int);
    int ret = -1;
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
    struct Data *darg = (struct Data*)arg;
    char filename[6][20] = {"cpu.log", "disk.log", "mem.log", "user.log", "sys.log", "enermy.log"};
    while (1) { 
        sleep(15);
        Node *p = darg->head;
        while (p->next) {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                perror("socket");
                write_log(Error_master, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                p = p->next;
                continue;
            }
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(darg->ctlport);
            addr.sin_addr = p->next->addr.sin_addr;
            if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                close(sockfd);
                perror("connect");
                write_log(Error_master, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                p = p->next;
                continue;
            }
            for (int i = 100; i <= 105; i++) {
                int ret = send(sockfd, &i, sizeof(int), 0);
                if (ret < 0) {
                    perror("send");
                    write_log(Error_master, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    continue;
                }
                int ack = 0;
                int rek = recv(sockfd, &ack, sizeof(int), 0);
                if (rek <= 0) {
                    perror("recv");
                    write_log(Error_master, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    continue;
                }
                if (!ack) continue;
                
                int recvfd = socket_connect(darg->dataport, inet_ntoa(p->next->addr.sin_addr));
                if(recvfd < 0) {
                    perror("socket_connet");
                    write_log(Error_master, "[数据模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    continue;
                }
                char fdir[100] = {0};
                sprintf(fdir, "%s/%s", mpath, inet_ntoa(p->next->addr.sin_addr));
                if (access(fdir, F_OK) < 0) {
                    int status = mkdir(fdir, 0755);
                    if (status < 0) {
                        perror("mkdir");
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
    int *inarg = (int *)arg;
    int port = *inarg;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        write_log(Error_master, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        return NULL;
    }
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        write_log(Error_master, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        close(sockfd);
        return NULL;
    }
    struct sockaddr_in addr, claddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        write_log(Error_master, "[警报模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
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

void listen_epoll(int listenfd, LinkList *linklist, int *sum, int ins, int heartPort) {
    unsigned long ul = 1;
    ioctl(listenfd, FIONBIO, &ul);
    int epollfd = epoll_create(MAXCLIENT);
    if (epollfd < 0) {
        perror("epoll_create");
        write_log(Error_master, "[监听模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        exit(1);
    }
    struct epoll_event ev, events[MAXCLIENT];
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        perror("epoll_ctl");
        write_log(Error_master, "[监听模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
        exit(1);
    }
    while (1) {
        int reval = epoll_wait(epollfd, events, MAXCLIENT, -1);
        if (reval < 0) {
            perror("epoll_wait");
            write_log(Error_master, "[监听模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
            exit(1);
        } else if (reval == 0) {
            continue;
        }
        for (int i = 0; i < reval; i++) {
            if (events[i].data.fd == listenfd && events[i].events & EPOLLIN) {
                struct sockaddr_in addr;
                socklen_t len = sizeof(addr);
                int newfd = accept(listenfd, (struct sockaddr*)&addr, &len);
                if (newfd < 0) {
                    perror("accept");
                    write_log(Error_master, "[监听模块] [error] [process : %d] [message : %s]", getpid(), strerror(errno));
                    exit(1);
                }
                int sub = find_min(sum, ins);
                Node *p = (Node *) malloc (sizeof(Node));
                addr.sin_port = htons(heartPort);
                p->fd = newfd;
                p->addr = addr;
                p->next = NULL;
                insert(linklist[sub], p);
                sum[sub] += 1;
                printf("someone come in\n");
                close(newfd);
            }
        }
    }
    close(listenfd);
}




