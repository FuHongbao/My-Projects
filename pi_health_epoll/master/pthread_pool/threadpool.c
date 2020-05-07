/************************************************************************
	> File Name: threadpool.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月25日 星期六 19时30分08秒
 ************************************************************************/

#include "threadpool.h"

void *thread_routine(void *arg) {
    struct timespec stime;
    threadpool_t *pool = (threadpool_t*)arg;
    int timeout = 0;
    while (1) {
        pthread_mutex_lock(&pool->mutex);
        pool->idle += 1;
        while (pool->first == NULL && !pool->quit) {
            stime.tv_sec += 2;
            timeout = pthread_cond_timedwait(&pool->not_empty, &pool->mutex, &stime);
            if (timeout == ETIMEDOUT) {
                break;
            }
        }
        pool->idle -= 1;
        if (pool->quit || timeout == ETIMEDOUT) {
            pthread_mutex_unlock(&pool->mutex);
            pthread_exit(NULL);
        } 
        if (pool->first) {
            task_t *t = pool->first;
            pool->first = t->next;
            pool->count ++;
            pthread_mutex_unlock(&pool->mutex);
            t->run(t->argv);
            pthread_mutex_lock(&pool->mutex);
            pool->count --;
            if (pool->count == 0 && pool->quit) {
                pthread_cond_signal(&pool->all_over);
            }
            pthread_mutex_unlock(&pool->mutex);
            free(t);
        }
    }
    return NULL;
}

int is_thread_alive(pthread_t pth) {
    int kill_rc = pthread_kill(pth, 0);
    if (kill_rc == ESRCH) {
        return 0;
    }
    return 1;
}

int thread_add_task(threadpool_t *pool, void *(*run)(void *args), void *argv) {
    task_t *newtask = (task_t*) malloc(sizeof(task_t));
    newtask->run = run;
    newtask->argv = argv;
    newtask->next = NULL;
    
    pthread_mutex_lock(&pool->mutex);
    if (pool->first == NULL) {
        pool->first = newtask;
    } else {
        pool->last->next = newtask;
    }
    pool->last = newtask;
    if (pool->quit) {
        pthread_mutex_unlock(&pool->mutex);
        return -1;
    }
    if (pool->idle > 0) {
        pthread_cond_signal(&pool->not_empty);
    } else if (pool->count < pool->max_threads) {
        int cnt = 0;
        for (int i = 0; i < pool->max_threads; i++) {
            int ret = is_thread_alive(pool->pth[i]);
            if (!ret) {
                pthread_create(&pool->pth[i], NULL, thread_routine, pool);
                cnt++;
            } 
            if (cnt == DEFAULTADDCOUNT) break;
        }
    } 
    pthread_mutex_unlock(&pool->mutex);
    return 0;
}


void threadpool_init(threadpool_t *pool, int max_threads) {
    pool->first = pool->last = NULL;
    pool->count = 0;  //目前正在工作的线程数
    pool->idle = 0;
    pool->max_threads = max_threads;
    pool->quit = 0;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pthread_cond_init(&pool->all_over, NULL);
    pool->pth = (pthread_t *) malloc(sizeof(pthread_t) * max_threads);
    for (int i = 0; i < max_threads; i++) {
        pthread_create(&pool->pth[i], NULL, thread_routine, pool);
    }
}


void threadpool_destroy(threadpool_t *pool) {
    if (pool->quit) return ;
    pthread_mutex_lock(&pool->mutex);
    pool->quit = 1;
    if (pool->count == 0) {
        pthread_cond_signal(&pool->not_empty);
    }
    if (pool->count > 0) {
        pthread_cond_wait(&pool->all_over, &pool->mutex);
    }
    for (int i = 0; i < pool->max_threads; i++) {
        pthread_join(pool->pth[i], NULL);
    }
    free(pool->pth);
    task_t *del;
    while (pool->first) {
        del = pool->first;
        pool->first = pool->first->next;
        free(del);
    }
    pthread_mutex_unlock(&pool->mutex);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->all_over);
    pthread_mutex_destroy(&pool->mutex);
    return ;
} 

