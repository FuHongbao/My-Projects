/*************************************************************************
	> File Name: common.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月25日 星期六 18时29分51秒
 ************************************************************************/

#include "common.h"

char *load_config(char *file, char *key) {
    if (file == NULL || key == NULL) return "";
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        return "";
    }
    size_t len = 0, n;
    ssize_t read = 0;
    char *ret_value = (char*) calloc(sizeof(char), 100);
    char *line = NULL, *substr = NULL;
    while ((read = getline(&line, &n, fp)) >= 0) {
        substr = strstr(line, key);
        if (substr == NULL) continue;
        len = strlen(key);
        if (line[len] != '=') continue;
        strncpy(ret_value, line + len + 1, (int)read - len - 2);
        break;
    }
    free(line);
    fclose(fp);
    return ret_value;
}

int write_log(char *path, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    time_t timep;
    time(&timep);
    struct tm *p;
    p = gmtime(&timep);
    FILE *fp = fopen(path, "a+");
    fprintf(fp, "[%d年-%d月-%d日 ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);
    fprintf(fp, "%d时-%d分-%d秒]", (8 + p->tm_hour), p->tm_min, p->tm_sec);
    int ret = vfprintf(fp, format, arg);
    fprintf(fp, "\n");
    fclose(fp);
    va_end(arg);
    return ret;
}

int socket_create_tcp(int port) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    int flag = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        close(socketfd);
        return -1;
    }
    struct sockaddr_in addr;
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

int socket_connect_tcp(int port, char *ip) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(socketfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(socketfd);
        return -1;
    }
    return socketfd;
}



