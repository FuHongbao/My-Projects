/*************************************************************************
	> File Name: client.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年03月31日 星期二 20时02分07秒
 ************************************************************************/

#include <stdio.h>
#include "client.h"


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

    int heartPort, dataPort, loadPort, ctlPort, warnPort;
    char tmp[20] = {0};
    char *config = "/opt/pi_client/client.conf";
    char ip[20] = {0}; 

    load_config(config, "IP", tmp);
    strcpy(ip, tmp);
    memset(tmp, 0, sizeof(tmp));

    load_config(config, "HeartPort", tmp);
    heartPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));

    load_config(config, "DataPort", tmp);
    dataPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));

    load_config(config, "LoadPort", tmp);
    loadPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    load_config(config, "CtlPort", tmp);
    ctlPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    load_config(config, "WarnPort", tmp);
    warnPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    if (is_use(heartPort) || is_use(ctlPort)) {
        write_log(errorLogPath, "[重复开启] [error] [process : %d] [message : the process has running]", getpid()); 
        exit(0);
    } 
   
    pthread_mutex_init(&smutex, NULL);
    pthread_cond_init(&scond, NULL);
    
    pthread_t pth_data;
    struct PthDataTrans data_arg;
    data_arg.dataPort = dataPort;
    data_arg.ctlPort = ctlPort;
    pthread_create(&pth_data, NULL, do_data, &data_arg);
   
    pthread_t pth_heart;
    struct PthHeartCheck heart_arg;
    heart_arg.heartPort = heartPort;
    pthread_create(&pth_heart, NULL, do_heart, &heart_arg);
   
    pthread_t pth_check;
    struct PthHealthCheck health_arg[6];
    for (int i = 0; i < 6; i++) {
        health_arg[i].inx = i;
        health_arg[i].warnPort = warnPort;
        health_arg[i].ip = ip;
        pthread_create(&pth_check, NULL, do_check, &health_arg[i]);
    } 
    while (1) {
        pthread_mutex_lock(&smutex);
        pthread_cond_wait(&scond, &smutex);
        pthread_mutex_unlock(&smutex);

        do_load(ip, loadPort);

        pthread_mutex_lock(&smutex);
        shareCnt = 0; 
        pthread_mutex_unlock(&smutex);
    } 
    
    return 0;
}

