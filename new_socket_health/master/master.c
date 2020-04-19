/*************************************************************************
	> File Name: master.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年03月29日 星期日 17时18分30秒
 ************************************************************************/

#include <stdio.h>
#include "master.h"

int main() {
    pid_t pid = fork();
    if (pid > 0) {
        exit(0);
    }
    setsid();
    umask(0);
    for (int i = 0; i < NR_OPEN; i++) {
        close(i);
    }
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    chdir("/");

    char *config = "/opt/pi_master/master.conf";
    char start_ip[20] = {0}, end_ip[20] = {0};
    int ins, heart_port, data_port, listen_port, ctl_port, warn_port;
    char temp_val[20] = {0};
    long timeout;
    load_config(config, "INS", temp_val);
    ins = atoi(temp_val);
    memset(temp_val, 0, sizeof(temp_val));
    
    load_config(config, "HeartPort", temp_val);
    heart_port = atoi(temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "DataPort", temp_val);
    data_port = atoi(temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "ListenPort", temp_val);
    listen_port = atoi(temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "CtlPort", temp_val);
    ctl_port = atoi(temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "WarnPort", temp_val);
    warn_port = atoi(temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "StartIp", temp_val);
    strcpy(start_ip, temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "EndIp", temp_val);
    strcpy(end_ip, temp_val);
    memset(temp_val, 0, sizeof(temp_val));

    load_config(config, "TimeOut", temp_val);
    timeout = atol(temp_val);
    
    LNode *link_list = (LNode*) malloc (sizeof(LNode) * ins);
    int *sum = (int*) malloc (sizeof(int) * ins);
    memset(sum, 0, sizeof(int)*ins);

    //初始化链表虚拟头结点
    struct sockaddr_in init_addr;
    init_addr.sin_family = AF_INET;
    init_addr.sin_port = htons(0);
    init_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    for (int i = 0; i < ins; i++) {
        LNode p = (LNode) malloc (sizeof(Node));
        p->addr = init_addr;
        p->fd = -1;
        p->next = NULL;
        link_list[i] = p;
    }
    unsigned int sip, eip;
    sip = ntohl(inet_addr(start_ip));
    eip = ntohl(inet_addr(end_ip));
    for (unsigned int i = sip; i <= eip; i++) {
        if (i % 256 == 0 || i % 256 == 255) continue;
        LNode p = (LNode) malloc (sizeof(Node));
        init_addr.sin_port = htons(heart_port);
        init_addr.sin_addr.s_addr = htonl(i);
        p->addr = init_addr;
        p->fd = -1;
        p->next = NULL;
        int ind = find_min(sum, ins);
        int ret = insert(link_list[ind], p);
        if (ret != 0) {
            write_log(errorLogPath, "failed to init node list(%d)", ind);
            return 1;
        }
        sum[ind] += 1;
    }
    pthread_t pth_heart, pth_data[ins];
    struct PthHeartCheck heart;
    heart.ins = ins;
    heart.list = link_list;
    heart.sum = sum;
    heart.timeout = timeout;
    pthread_create(&pth_heart, NULL, do_heart, (void*)&heart);

    struct PthDataTrans darg[ins];
    for (int i = 0; i < ins; i++) {
        darg[i].head = link_list[i];
        darg[i].ind= i;
        darg[i].dataPort = data_port;
        darg[i].ctlPort = ctl_port;
        pthread_create(&pth_data[i], NULL, do_data, (void*)&darg[i]);
    }
    
    pthread_t pth_warn;
    pthread_create(&pth_warn, NULL, do_warn, (void*)&warn_port);

    int listenfd = socket_listen(listen_port);
    int ret = listen_client(listenfd, link_list, sum, ins, heart_port);
    if (ret < 0) {
        close(listenfd);
        return 0;
    } 
    for (int i = 0; i < ins; i++) {
        pthread_join(pth_data[i], NULL);
    }
    pthread_join(pth_heart, NULL);
    return 0;
}



