/*************************************************************************
	> File Name: my_ls.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年04月28日 星期日 22时13分44秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <time.h>

void s_mode (int mode, char *res) {

    strcpy(res,"----------");
    if (S_ISDIR (mode)) {res[0] = 'd';}
    if (S_ISCHR (mode)) {res[0] = 'c';}
    if (S_ISBLK (mode)) {res[0] = 'b';}
    if (S_ISLNK (mode)) {res[0] = 'l';}
    if (S_ISFIFO (mode)) {res[0] = 'p';}
    if (mode & S_IRUSR)  {res[1] = 'r';}
    if (mode & S_IWUSR)  {res[2] = 'w';}
    if (mode & S_IXUSR)  {res[3] = 'x';}
    if (mode & S_ISUID)  {res[3] = 's';}
    if (mode & S_IRGRP)  {res[4] = 'r';}
    if (mode & S_IWGRP)  {res[5] = 'w';}
    if (mode & S_IXGRP)  {res[6] = 'x';}
    if (mode & S_ISGID)  {res[6] = 's';}
    if (mode & S_IROTH)  {res[7] = 'r';}
    if (mode & S_IWOTH)  {res[8] = 'w';}
    if (mode & S_IXOTH)  {res[9] = 'x';}
    if (mode & S_ISVTX)  {res[9] = 't';}
}

void showinfo(char *name, int index) {
    struct stat st;
    struct passwd *pgd,*pud;
    if (stat(name, &st) < 0) {
        perror("stat");
        return ;
    }
    char mode[10],*s_time;
    s_time = ctime(&st.st_mtime);
    int len = strlen(s_time);
    s_time[len-1] = '\0';
    if (index) {
        s_mode(st.st_mode, mode);
        pud = getpwuid (st.st_uid);
        pgd = getpwuid (st.st_gid);
        printf ("%s", mode);
        printf ("%4ld", (long)st.st_nlink);
        printf ("%10s", pud->pw_name);
        printf ("%10s", pgd->pw_name);
        printf ("%8ld", (long)st.st_size);
        printf ("%20.16s", s_time);
        printf ("\t%s\n", name);
    } else {
        if(name[0] != '.')
            printf("%s\t", name);
    }
}

void do_ls(char *arg, int index) {
    DIR *dir;
    struct stat st;
    stat(arg,&st);
    int is_dir = 0;
    if (S_ISDIR (st.st_mode)) {
        is_dir = 1;
    }
    if(is_dir) {
        char buff[200];
        getcwd (buff,sizeof(buff));
        chdir(arg);  //进入目录
        if ((dir = opendir (arg)) == NULL) {
            perror ("opendir");
            return ;
        }
        struct dirent *redir;
        while ((redir = readdir (dir)) != NULL) {
            showinfo (redir->d_name, index);
        }
        if(!index)
        printf("\n");
        closedir(dir); //关闭dir
        chdir(buff); //返回目录
    } else {
        showinfo (arg, index);
        printf("\n");
    }
}

void my_ls (int n, char *arg[]) {
    int index = 0;
    if (n == 1) {
        char s[10];
        strcpy (s, ".");
        do_ls(s, index);
    } else {
        if(n == 2 && !strcmp(arg[1], "-al")) {
            char s[10];
            index = 1;
            strcpy(s, ".");
            do_ls(s, index);
        } else {
            for (int i = 1; i < n; i++) {
                if (i == 1 && !strcmp(arg[i], "-al")) { 
                    index = 1;
                    continue;
                }
                do_ls (arg[i], index);
            }
        }
    }
}







