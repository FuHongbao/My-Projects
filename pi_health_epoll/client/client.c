/*************************************************************************
	> File Name: client.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月28日 星期二 21时48分25秒
 ************************************************************************/

#include "client.h"

char *conf_path = "./client.conf";
char *num_req[FILE_N] = {"Req_0", "Req_1", "Req_2", "Req_3", "Req_4", "Req_5"};
char *num_ack[FILE_N] = {"Ack_0", "Ack_1", "Ack_2", "Ack_3", "Ack_4", "Ack_5"};
char *filename[FILE_N] = {"file_0", "file_1", "file_2", "file_3", "file_4", "file_5"};
char *shellname[FILE_N] = {"shell_0", "shell_1", "shell_2", "shell_3", "shell_4", "shell_5"};



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
        task_obj[i].shellname = load_config(conf_path, shellname[i]);
    }
    conf.master_port = atoi(load_config(conf_path, "Master_Port"));
    conf.trans_port = atoi(load_config(conf_path, "Trans_Port"));
    conf.ctrl_port = atoi(load_config(conf_path, "Ctrl_Port"));
    conf.data_port = atoi(load_config(conf_path, "Data_Port"));
    conf.warn_port = atoi(load_config(conf_path, "Warn_Port"));
    conf.time_sec = atol(load_config(conf_path, "Time_Sec"));
    conf.time_usec = atol(load_config(conf_path, "Time_Usec"));
    conf.ip = load_config(conf_path, "Master_IP");
     
    pthread_mutex_init(&smutex, NULL);
    pthread_cond_init(&scond, NULL);
    
    pthread_t pth_data, pth_heart, pth_bash;
    
    pthread_create(&pth_heart, NULL, do_heart, NULL); 
    pthread_create(&pth_data, NULL, do_data, NULL);
    pid_t pid_1 = fork();
    if (pid_1 < 0) {
        perror("fork");
        exit(1);
    }
    if (pid_1 == 0) {
        int inx[6]; 
        for (int i = 0; i < 6; i++) {
            inx[i] = i;
            pthread_create(&pth_bash, NULL, do_bash, (void*)&inx[i]);
        }
        for (int i = 0; i < 6; i++) {
            pthread_join(pth_bash, NULL); 
        }
    } else {
        pthread_join(pth_heart, NULL);
        pthread_join(pth_data, NULL);
        wait(&pid_1);
    }
    return 0;
}


