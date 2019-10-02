/*************************************************************************
	> File Name: client.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年08月02日 星期五 16时45分03秒
 ************************************************************************/

#include <stdio.h>
#include "client.h"


int main() {
    int heartPort, dataPort, loadPort;
    char tmp[20] = {0};
    char *config = "./conf_client.log";
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
    
    pid_t pid;
    pid = fork();
    if (pid > 0) {
        recv_data(dataPort);
        wait(&pid); 
    } else {
        recv_heart(heartPort);
    }
    
    return 0;
}
