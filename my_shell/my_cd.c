/*************************************************************************
	> File Name: my_cd.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年04月28日 星期日 21时29分19秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void my_cd (int n, char *str[]) {
    if(n > 2) {
        printf("错误信息：参数格式错误....\n");
        return ;
    }
    char p[100];
    if(n == 1){
        memset(p, '\0', sizeof(p));
        struct passwd *pwd;
        pwd = getpwuid(getuid());
        sprintf (p, "/home/%s", pwd->pw_name);
    } else {
        sprintf (p, str[1]);
    } 
    int res = chdir(p);
    if(res != 0) printf("错误信息：无效路径....\n");
    return ;
}




