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
    int port;
};


int insert (LinkList head, Node *node);
void output(LinkList head);
int find_min(int *sum, int ins) ;
void *do_heart(void *arg);
void *do_data(void *arg);
int check_connect(struct sockaddr_in addr, long timeout);
void listen_epoll(int listenfd, LinkList *linklist, int *sum, int ins, int heartPort);


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
    char log[50] = {0};
    sprintf(log, "./pth_%d.log", darg->ind);
    int epollfd = epoll_create(MAXCLIENT);
    struct epoll_event ev, events[MAXCLIENT];
    while (1) {
        FILE *fp = fopen(log, "a+");
        Node *p = darg->head;
        while (p->next) {
            int askfd = socket(AF_INET, SOCK_STREAM, 0);
            if (askfd < 0) {
                perror("Error socket on askfd");
                p = p->next;
                continue;
            }
            unsigned long ul = 1;
            ioctl(askfd, FIONBIO, &ul);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(darg->port);    
            addr.sin_addr = p->next->addr.sin_addr;
            connect(askfd, (struct sockaddr*)&addr, sizeof(addr));
            fprintf(fp, "%s : %d login\n", inet_ntoa(addr.sin_addr), darg->port);
            ev.events = EPOLLOUT;
            ev.data.fd = askfd;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, askfd, &ev);
            p->fd = askfd;
            p = p->next;
        }
        int reval = epoll_wait(epollfd, events, MAXCLIENT, 3*1000);
        if (reval < 0) {
            perror("epoll_wait");
            fclose(fp);
            return NULL;
        } else if (reval == 0) {
            sleep(5);
            fclose(fp);
            sleep(5);
            continue;
        }
        for (int i = 0; i < reval; i++) {
            int clientfd = events[i].data.fd;
            if (events[i].events & EPOLLOUT) {
                unsigned long ul = 0;
                ioctl(clientfd, FIONBIO, &ul);
                char buff[BUFFSIZE] = {0};
                strcpy(buff, "hello");
                send(clientfd, buff, strlen(buff), 0);
                memset(buff, 0, sizeof(buff));
                int k = recv(clientfd, buff, sizeof(buff), 0);
                if (k > 0) {
                    printf("%s\n", buff);
                }
            }
            struct epoll_event em;
            em.data.fd = clientfd;
            epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, &em);
            close(clientfd);
        }
        fclose(fp);
        sleep(5);
    }
}

void listen_epoll(int listenfd, LinkList *linklist, int *sum, int ins, int heartPort) {
    unsigned long ul = 1;
    ioctl(listenfd, FIONBIO, &ul);
    int epollfd = epoll_create(MAXCLIENT);
    if (epollfd < 0) {
        perror("epoll_create");
        return ;
    }
    struct epoll_event events[MAXCLIENT], ev;    
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        perror("epoll_ctl");
        return ;
    }
    while (1) {
        int reval = epoll_wait(epollfd, events, MAXCLIENT, -1);
        if (reval < 0) {
            perror("epoll_wait");
            close(listenfd);
            return ;
        } else if (reval == 0){
            continue;
        }
        
        for (int i = 0; i < reval; i++) {
            if (events[i].data.fd == listenfd && events[i].events & EPOLLIN) {
                struct sockaddr_in addr;
                socklen_t len = sizeof(addr);
                int newfd = accept(listenfd, (struct sockaddr*)&addr, &len);
                if (newfd < 0) {
                    perror("accept");
                    continue;
                } 
                Node *p = (Node *) malloc (sizeof(Node));
                addr.sin_port = htons(heartPort);
                p->fd = newfd;
                p->next = NULL;
                int sub = find_min(sum, ins);
                insert(linklist[sub], p);
                sum[sub] += 1;
                printf("some join success\n");
                close(newfd);
            }
        }
    }
    close(listenfd); 
}




