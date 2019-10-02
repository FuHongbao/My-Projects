/*************************************************************************
	> File Name: my_shell.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年04月28日 星期日 20时30分31秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include "my_cd.h"
#include "my_ls.h"

char *str[200];
char ml[100][20] = {"cd","ls","cat","pwd"};
int cnt;         // 切割后的字符串个数

//切割命令
void cut_cmd(char *cmd) {
    memset(str, '\0', sizeof(str));
    int len = strlen(cmd);
    cmd[len-1] = ' ';
    cnt = 0;
    str[cnt] = strtok(cmd, " ");
    while(str[cnt] != NULL) {
        cnt++;
        str[cnt] = strtok(NULL," ");
    }
}

//判断命令并执行
int wch_cmd(){
    if (!strcmp (str[0],"exit")) {
        return -1;
    }
    int index = -1;
    for (int i = 0; i < 100; i++) {
        if (!strcmp(str[0],ml[i])) {
            index = i;
            break;
        }
    }
    switch (index) {
        case 0:
            my_cd(cnt,str);
            break;
        case 1:
            my_ls(cnt,str);
            break;
        case 2:

            break;
        default :
            printf("未找到此命令....\n");
    }
    return 0;
}

//打印终端提示
void my_show(){
    //获取用户信息的结构体
    struct passwd *pwd;
    char host[100];
    char buff[100];
    pwd = getpwuid(getuid());  //通过uid获取信息
    gethostname (host,sizeof(host)); //获取主机名
    getcwd (buff, sizeof(buff));   //获取当前路径
    int len = strlen(buff);
    int len1 = strlen(pwd->pw_dir);
    char loc[2000];
    memset(loc,'\0',sizeof(loc));
    if(len >= len1) {
        loc[0] = '~';
        strncpy(loc+1, buff+len1, len-len1);
    } else {
        strcpy(loc,buff);
    }
    printf ("\033[34m%s@%s:",pwd->pw_name, host);
    if (!strcmp(pwd->pw_name,"root")){
        printf("%s# ",loc);
    } else {
        printf("%s$ ",loc);
    }
    fflush(stdout);
}

int main() {
    char cmd[1000];
    //屏蔽信号ctrl c
    signal (SIGINT, SIG_IGN);
    while (1) {
        my_show();
        //获取终端输入
        fgets(cmd,999,stdin);
        //消除回车影响
        if(strcmp(cmd,"\n")){
            cut_cmd(cmd);
            if (wch_cmd() < 0) {
                break;
            }

        }
    }
    return 0;
}


