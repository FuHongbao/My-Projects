/*************************************************************************
	> File Name: threadpool.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月25日 星期六 19时30分03秒
 ************************************************************************/

#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#define DEFAULTADDCOUNT 5
#endif

typedef struct task {
    void *(*run)(void *arg);
    void *argv;
    struct task *next;
} task_t;

typedef struct threadpool {
    pthread_cond_t not_empty; //任务队列有内容，可以获取线程执行
    pthread_cond_t all_over; //任务队列未满，可以添加
    pthread_mutex_t mutex;
    task_t *first, *last;
    pthread_t *pth;
    int count, idle;  //idle为空闲线程数
    int max_threads;  //最大线程数
    int min_threads;  //最小线程数
    int quit;         //退出标识
} threadpool_t;

threadpool_t pool;
void threadpool_init(threadpool_t *pool, int max_threads);
int thread_add_task(threadpool_t *pool, void *(*run)(void *args), void *argv);
void threadpool_destroy(threadpool_t *pool);

