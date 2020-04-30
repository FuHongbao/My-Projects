/*******************************************************************mZ**
	> File Name: master.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月26日 星期日 14时27分40秒
 ************************************************************************/

#include "./epoll/epoll.h"

char *num_req[FILE_N] = {"Req_0", "Req_1", "Req_2", "Req_3", "Req_4", "Req_5"};
char *num_ack[FILE_N] = {"Ack_0", "Ack_1", "Ack_2", "Ack_3", "Ack_4", "Ack_5"};
char *filename[FILE_N] = {"file_0", "file_1", "file_2", "file_3", "file_4", "file_5"};
char *conf_path = "./master.conf";
char *warn_log = "./warn.log";
int listenfd, datafd;


void *listen_epoll(void *arg) {
    unsigned long ul = 1;
    ioctl(listenfd, FIONBIO, &ul);
    ioctl(datafd, FIONBIO, &ul);
    do_epoll(table, listenfd, datafd);
    return NULL;
}

void *listen_warn(void *arg) {
    int warnfd = socket_create_udp(conf.warn_port); 
    if (warnfd < 0) {
        write_log(error_log, "[警报模块] [error] [message : create socket failed]");
        exit(1);
    }
    struct sockaddr_in cltaddr;
    socklen_t len = sizeof(cltaddr);
    char buff[MAXBUFFSIZE];
    while (true) {
        memset(buff, 0, sizeof(buff));
        recvfrom(warnfd, buff, sizeof(buff), 0, (struct sockaddr*)&cltaddr, &len);
        printf("recv warn data [%s]\n", buff);
        FILE *fp = fopen(warn_log, "a+");
        fprintf(fp, "%s-", buff);
        fclose(fp);
    }
    close(warnfd);
    return NULL;
}

void check(HashFind *table) {
    pthread_mutex_lock(&heart_mutex);
    for (int i = 0; i < table->cnt; i++) {
        LinkList p = table->arr[i];
        while (p->next) {
            if (p->next->cnt > 2) {
                LinkList q = p->next;
                p->next = q->next;
                delete_event(epollfd, q->fd, EPOLLIN);
                close(q->fd);
                free(q);
            } else {
                p->next->cnt ++;
                if (p->next) p = p->next;
            }
        }
    }
    pthread_mutex_unlock(&heart_mutex);
}

void heart_check() {
    while (true) {
        sleep(5);
        check(table);
        output(table); 
    }
}
int main() {
    pid_t pid = fork();
    if (pid > 0) {
        exit(0);
    }
    setsid();
    umask(0);
    int maxfd = sysconf(_SC_OPEN_MAX);
    for (int i = 0; i < maxfd; i++) {
        close(i);
    }
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    //chdir("/");

    for (int i = 0; i < FILE_N; i++) {
        task_obj[i].Req_num = atoi(load_config(conf_path, num_req[i]));
        task_obj[i].Ack_num = atoi(load_config(conf_path, num_ack[i]));
        task_obj[i].filename = load_config(conf_path, filename[i]);
    }
    conf.listen_port = atoi(load_config(conf_path, "Listen_Port"));
    conf.trans_port = atoi(load_config(conf_path, "Trans_Port"));
    conf.ctrl_port = atoi(load_config(conf_path, "Ctrl_Port"));
    conf.data_port = atoi(load_config(conf_path, "Data_Port"));
    conf.warn_port = atoi(load_config(conf_path, "Warn_Port"));
    conf.Start_Ip = load_config(conf_path, "Start_IP");
    conf.End_Ip = load_config(conf_path, "End_IP");
    conf.INS = atoi(load_config(conf_path, "INS"));

    table = init_hash();
    
    listenfd = socket_create_tcp(conf.listen_port); 
    if (listenfd < 0) {
        printf("failed to create listen socketfd\n");
        write_log(error_log, "failed to create listenfd, error = [%s]", strerror(errno));
        exit(1);
    }
    datafd = socket_create_tcp(conf.ctrl_port); 
    if (datafd < 0) {
        printf("failed to create data socketfd\n");
        write_log(error_log, "failed to create datafd, error = [%s]", strerror(errno));
        exit(1);
    }

    threadpool_init(&pool, conf.INS);    
    thread_add_task(&pool, listen_epoll, 0);
    thread_add_task(&pool, listen_warn, 0); 
    heart_check();
    
    return 0;
}

