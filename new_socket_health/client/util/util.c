/*************************************************************************
	> File Name: util.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年03月29日 星期日 17时20分34秒
 ************************************************************************/

#include <stdio.h>
#include "util.h"


int write_log(char *path, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    time_t timep;
    time(&timep);
    struct tm *p;
    p = gmtime(&timep);
    FILE *fp = fopen(path, "a+");
    fprintf(fp, "[%d年-%d月-%d日 ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);
    fprintf(fp, "%d时-%d分-%d秒]", (p->tm_hour + 8), p->tm_min, p->tm_sec);
    int k = vfprintf(fp, format, arg);
    fprintf(fp, "\n");
    fclose(fp);
    va_end(arg);
    return k;
}

int load_config(char *file, char *key, char *value) {
    if (key == NULL || value == NULL) {
        return -1;
    }
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
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
        strncpy(value, line + len + 1, (int)read - len - 2);
        break;
    }
    free(line);
    if (value == NULL) {
        return -1;
    }
    fclose(fp);
    return 0;
}

int socket_listen(int port) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    int flag = 1, len = sizeof(flag);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) < 0) {
        close(socketfd);
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socketfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(socketfd);
        return -1;
    }
    listen(socketfd, 10000);
    return socketfd;
}

int socket_connect(int port, char *ip) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(socketfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return -1;
    }
    return socketfd;
}



