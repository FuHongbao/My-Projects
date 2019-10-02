/*************************************************************************
	> File Name: util.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年08月02日 星期五 10时43分49秒
 ************************************************************************/

#include <stdio.h>
#include "util.h"

int get_conf_value(char *file, char *key, char *value) {
    FILE *fp = NULL;
    if (key == NULL || value == NULL) {
        printf("failed to get value\n");
        return -1;
    }
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("open file failed\n");
        return -1;
    }
    ssize_t read;
    size_t n, len;
    char *line = NULL, *substr = NULL;
    while ((read = getline(&line, &n, fp)) >= 0) {
        substr = strstr(line, key);
        if (substr == NULL) continue;
        len = strlen(key);
        if (line[len] != '=') continue;
        strncpy(value, line + len + 1, (int)read- len - 2);
        break;
    }
    if (value == NULL) {
        printf("not found key\n");
        return -1;
    }
    free(line);
    fclose(fp);
    return 0;
}

int socket_create(int port) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    int flag = 1, len = sizeof(flag);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) < 0) {
        close(socketfd);
        perror("setsockopt");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("ERROR bind on socketfd");
        return -1;
    }
    listen(socketfd, 2000);
    return socketfd;
}

int socket_connect(int port, char *ip) {
    int socketfd;    
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("failed to socket\n");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(socketfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("failed to connect\n");
        return -1;
    }
    return socketfd;
}

int write_log(char *path, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    time_t timep;
    time(&timep);
    struct tm *p;
    p = gmtime(&timep);
    FILE *fp = fopen(path, "a+");
    fprintf(fp, "[%d年-%d月-%d日 ", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday);
    fprintf(fp, "%d时-%d分-%d秒]", (p->tm_hour+8), p->tm_min, p->tm_sec);
    int k = vfprintf(fp, format, arg);
    fprintf(fp, "\n");
    fclose(fp);
    va_end(arg);
    return k;
}


