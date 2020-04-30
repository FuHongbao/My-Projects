/*************************************************************************
	> File Name: epoll.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月26日 星期日 11时20分00秒
 ************************************************************************/

#include "epoll.h"

void add_event(int epollfd, int sockfd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
}

void delete_event(int epollfd, int sockfd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &ev);
}

void modify_event(int epollfd, int sockfd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev);
}

void do_epoll(HashFind *table, int listenfd, int transfd) {
    struct epoll_event events[MAXEVENTS];
    int ret;
    epollfd = epoll_create(MAXEVENTS);
    add_event(epollfd, listenfd, (EPOLLIN | EPOLLET));
    add_event(epollfd, transfd, (EPOLLIN | EPOLLET));
    while (true) {
        ret = epoll_wait(epollfd, events, MAXEVENTS, -1);
        handle_events(table, epollfd, events, ret, listenfd, transfd);
    }
    close(epollfd);
    return;
}

void *Handle_Data(void *arg) {
    Epoll_Param *param = (Epoll_Param*)arg;
    int fd = param->sockfd;
    int epollfd = param->epollfd;
    HashFind *table = param->table;
    struct sockaddr_in faddr;
    socklen_t len = sizeof(faddr);
    int newfd = accept(fd, (struct sockaddr*)&faddr, &len);
    if (newfd < 0) {
        perror("accept");
        printf("failed to build connect, sockfd=%d, ip=%s\n", fd, inet_ntoa(faddr.sin_addr));
        free(param);
        return NULL;
    }
    do_data(newfd, inet_ntoa(user_msg[param->sockfd].addr.sin_addr));
    free(param);
    close(newfd); 
}

void handle_events(HashFind *table, int epollfd, struct epoll_event *events, int num, int listenfd, int transfd) {
    for (int i = 0; i < num; i++) {
        int fd = events[i].data.fd;
        Epoll_Param *ep = (Epoll_Param*) malloc (sizeof(Epoll_Param));
        ep->table = table;
        ep->epollfd = epollfd;
        ep->sockfd = fd;
        if (events[i].events & EPOLLIN) {
            if (fd == listenfd) {
                thread_add_task(&pool, Handle_Accept, (void*)ep);
                continue;
            }
            if (fd == transfd) {
                thread_add_task(&pool, Handle_Data, (void*)ep);
                continue;
            }
            thread_add_task(&pool, Handle_Read,(void*)ep);
        } else if (events[i].events & EPOLLOUT) {
            thread_add_task(&pool, Handle_Write,(void*)ep);
        } 
    }
    return;
}

void *Handle_Accept(void *arg) {
    Epoll_Param *param = (Epoll_Param*)arg;
    int fd = param->sockfd;
    int epollfd = param->epollfd;
    HashFind *table = param->table;
    struct sockaddr_in faddr;
    socklen_t len = sizeof(faddr);
    int newfd = accept(fd, (struct sockaddr*)&faddr, &len);
    if (newfd < 0) {
        perror("accept");
        printf("failed to build connect, sockfd=%d, ip=%s\n", fd, inet_ntoa(faddr.sin_addr));
        free(param);
        return NULL;
    }
    if (!look_ip(table, faddr)) {
        Node *node= node_create(faddr, newfd);
        table = insert_node(table ,node);
        user_msg[newfd].addr = faddr;
    }
    add_event(epollfd, newfd, EPOLLIN | EPOLLET);
    free(param);
}

void do_data(int ctrlfd, char *ip) {
    for  (int i = 0; i < 6; i++) {
        int ret = send(ctrlfd, &task_obj[i].Req_num, sizeof(int), 0);
        if (ret < 0) {
            perror("send");
            write_log(error_log, "[数据模块] [error] [message : %s]", strerror(errno));
            continue;
        }
        int ack = 0;
        ret = recv(ctrlfd, &ack, sizeof(int), 0);
        if (ret < 0) {
            perror("recv");
            write_log(error_log, "[数据模块] [error] [message : %s]", strerror(errno));
            continue;
        } else if (ret == 0) {
            break;
        }
        if (ack != task_obj[i].Ack_num) continue;
        int datafd = socket_connect_tcp(conf.data_port, ip);
        if (datafd < 0) {
            printf("failed to connect client(%s) on data port, resource num = [%d]\n", ip, task_obj[i].Req_num);
            write_log(error_log, "[数据模块] [error] [message : failed to connect client on data port]");
            continue;
        }
        char fdir[100] = {0};
        sprintf(fdir, "%s/%s", base_log, ip);
        if (access(fdir, F_OK) < 0) {
            int status = mkdir(fdir, 0755);
            if (status < 0) {
                perror("mkdir");
                close(datafd);
                continue;
            }
        }
        char filepath[100] = {0};
        sprintf(filepath, "%s/%s", fdir, task_obj[i].filename);
        char buff[MAXBUFFSIZE] = {0};
        FILE *fw = fopen(filepath, "a+");
        while (1) {
            int k = recv(datafd, buff, sizeof(buff), 0);
            if (k <= 0) break;
            fprintf(fw, "%s", buff);
            memset(buff, 0, sizeof(buff));
        }
        fclose(fw);
        close(datafd);
    } 
}


void reset_heart_cnt(struct sockaddr_in addr) {
    int inx = hash_func(addr);
    LinkList p = table->arr[inx];
    pthread_mutex_lock(&heart_mutex);
    while (p->next) {
        if (p->next->addr.sin_addr.s_addr == addr.sin_addr.s_addr) {
            p->next->cnt = 0; 
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&heart_mutex);
}

void *Handle_Read(void *arg) {
    Epoll_Param *param = (Epoll_Param*)arg;
    int cnt = recv(param->sockfd, user_msg[param->sockfd].buff, MAXBUFFSIZE, 0);
    if (cnt <= 0) {
        table = delete_node(table, user_msg[param->sockfd].addr);
        delete_event(param->epollfd, param->sockfd, EPOLLIN);
        close(param->sockfd);
        free(param);
        return NULL;
    } else {
        //TODO::处理接收信息
        if (!strcmp(user_msg[param->sockfd].buff, "heart")) {
            reset_heart_cnt(user_msg[param->sockfd].addr); 
            strcpy(user_msg[param->sockfd].buff, "ok"); 
            modify_event(param->epollfd, param->sockfd, (EPOLLOUT | EPOLLET));
        }
    } 
    free(param);
}

void *Handle_Write(void *arg) {
    Epoll_Param *param = (Epoll_Param*)arg;
    int cnt = send(param->sockfd, user_msg[param->sockfd].buff, strlen(user_msg[param->sockfd].buff), 0);
    memset(user_msg[param->sockfd].buff, 0, sizeof(user_msg[param->sockfd].buff));
    if (cnt < 0) {
        free(param);
        return NULL;
    } else {
        modify_event(param->epollfd, param->sockfd, (EPOLLIN | EPOLLET));
    }
    free(param);
}


