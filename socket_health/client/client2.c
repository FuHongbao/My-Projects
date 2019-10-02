/*************************************************************************
> File Name: client.c
> Author: victoria 
> Mail: 1105847344@qq.com
> Created Time: 2019年08月02日 星期五 16时45分03秒
************************************************************************/

#include <stdio.h>
#include "client2.h"

int main() {
    pid_t tpid = fork();
    if (tpid > 0) {
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

    get_conf_value(config, "IP", tmp);
    strcpy(ip, tmp);
    memset(tmp, 0, sizeof(tmp));

    get_conf_value(config, "HeartPort", tmp);
    heartPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));

    get_conf_value(config, "DataPort", tmp);
    dataPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));

    get_conf_value(config, "LoadPort", tmp);
    loadPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    get_conf_value(config, "CtlPort", tmp);
    ctlPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    get_conf_value(config, "WarnPort", tmp);
    warnPort = atoi(tmp);
    memset(tmp, 0, sizeof(tmp));
    
    if (is_use(heartPort) || is_use(ctlPort)) {
        write_log(Error_log, "[重复开启] [error] [process : %d] [message : the process has running]", getpid()); 
        exit(0);
    } 
    
    
    pthread_mutexattr_t mattr; 
    pthread_condattr_t cattr;
    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    
    struct Share *share = NULL;
    int shmid = shmget(IPC_PRIVATE, sizeof(struct Share), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        write_log(Error_log, "[共享内存] [error] [process : %d] [message : %s]", getpid(), strerror(errno)); 
        exit(1);
    } 
    share = (struct Share*) shmat (shmid, NULL, 0);
    if (share == (void *)-1) {
        perror("shmat");
        write_log(Error_log, "[共享内存] [error] [process : %d] [message : %s]", getpid(), strerror(errno)); 
        exit(1);
    }
    pthread_mutex_init(&share->smutex, &mattr);
    pthread_cond_init(&share->sready, &cattr);

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        write_log(Error_log, "[开辟进程] [error] [process : %d] [message : %s]", getpid(), strerror(errno)); 
    }
    if (pid > 0) {
        recv_data(dataPort, ctlPort, share);
        wait(&pid); 
    } else {
        pid_t pid_1;
        pid_1 = fork();
        if (pid_1 < 0) {
            write_log(Error_log, "[开辟进程] [error] [process : %d] [message : %s]", getpid(), strerror(errno)); 
        }
        if(pid_1 > 0) {
            recv_heart(heartPort, share);
            wait(&pid_1);
            exit(0);
        } else if(pid_1 == 0){
            pid_t pid_2;
            int inx;
            for (int i = 0; i < 6; i++) {
                inx = i;
                pid_2 = fork();
                if (pid_2 < 0) {
                    write_log(Error_log, "[开辟进程] [error] [process : %d] [message : %s]", getpid(), strerror(errno)); 
                }
                if (pid_2 == 0) break;
            }
            if (pid_2 > 0) {
                while (1) {
                    //条件触发
                    pthread_mutex_lock(&share->smutex);
                    pthread_cond_wait(&share->sready, &share->smutex); 
                    pthread_mutex_unlock(&share->smutex);

                    do_load(ip, loadPort, share);
                
                    pthread_mutex_lock(&share->smutex);
                    share->shareCnt = 0;
                    pthread_mutex_unlock(&share->smutex);

                }
                for (int i = 0; i < 6; i++) {
                    wait(&pid_2);
                }
                exit(0);
            }
            if (pid_2 == 0) {
                int cnt = 0;
                while(1) {
                    cnt += 1;
                    do_check(inx, share, cnt, warnPort, ip);
                    if (cnt == 5) cnt = 0;
                    sleep(2);
                } 
                exit(0);
            }
        }
    }

    return 0;
}
