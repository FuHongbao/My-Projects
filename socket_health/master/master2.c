/*************************************************************************
	> File Name: master.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年08月02日 星期五 11时31分37秒
 ************************************************************************/

#include <stdio.h>
#include "master2.h"

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
    char startIp[20] ={0}, endIp[20] = {0}; 
    int Ins, heartport, dataport, listenport, ctlport, warnport;
    char reval[20] = {0};
    long timeout;

    get_conf_value(config, "INS", reval);
    Ins = atoi(reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "HeartPort", reval);
    heartport = atoi(reval);
    memset(reval, 0, sizeof(reval));

    get_conf_value(config, "DataPort", reval);
    dataport = atoi(reval);
    memset(reval, 0, sizeof(reval));

    get_conf_value(config, "ListenPort", reval);
    listenport = atoi(reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "CtlPort", reval);
    ctlport = atoi(reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "WarnPort", reval);
    warnport = atoi(reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "StartIp", reval);
    strcpy(startIp, reval);
    memset(reval, 0, sizeof(reval));
    
    get_conf_value(config, "EndIp", reval);
    strcpy(endIp, reval);
    memset(reval, 0, sizeof(reval)); 
    
    get_conf_value(config, "TimeOut", reval);
    timeout = atol(reval);
    memset(reval, 0, sizeof(reval)); 
    
    LinkList *linklist = (LinkList*) malloc (sizeof(LinkList) * Ins);
    int *sum = (int*) malloc (sizeof(int) * Ins);
    memset(sum, 0, sizeof(int) * Ins); 
    
    struct sockaddr_in initaddr;
    initaddr.sin_family = AF_INET;
    initaddr.sin_port = htons(0);
    initaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    
    for (int i = 0; i < Ins; i++) {
        Node *p = (Node *) malloc (sizeof(Node));
        p->addr = initaddr;
        p->fd = -1;
        p->next = NULL;
        linklist[i] = p;
    }
    
    unsigned int sip, eip;
    sip = ntohl(inet_addr(startIp));
    eip = ntohl(inet_addr(endIp));
    for (unsigned int i = sip; i <= eip; i++) {
        if (i % 256 == 0 || i % 256 == 255) continue;
        Node *p = (Node*) malloc (sizeof(Node));
        initaddr.sin_port = htons(heartport);
        initaddr.sin_addr.s_addr = htonl(i);
        p->addr = initaddr;
        p->fd = -1;
        p->next = NULL;
        int sub = find_min(sum, Ins);
        insert(linklist[sub], p);
        sum[sub] += 1;
    }
    
    for (int i = 0; i < Ins; i++) {
        printf("<%d>\n", i);
        output(linklist[i]);
    }
    
    pthread_t pth_heart, pth_data[Ins];
    struct Heart heart;
    heart.ins = Ins;
    heart.linklist = linklist;
    heart.sum = sum;
    heart.timeout = timeout;
    pthread_create(&pth_heart, NULL, do_heart, (void*)&heart);
    
    struct Data darg[Ins];
    for (int i = 0; i < Ins; i++) {
        darg[i].head = linklist[i];
        darg[i].ind = i;
        darg[i].dataport = dataport;
        darg[i].ctlport = ctlport;
        pthread_create(&pth_data[i], NULL, do_data, (void*)&darg[i]);
    }
   
    pthread_t pth_warn;
    pthread_create (&pth_warn, NULL, do_warn, (void*)&warnport);

    int listenfd = socket_create(listenport);
    listen_epoll(listenfd, linklist, sum, Ins, heartport);
     
     
    for (int i = 0; i < Ins; i++) {
        pthread_join(pth_data[i], NULL);
    }
    pthread_join(pth_heart, NULL); 
    return 0;
}






