/*************************************************************************
	> File Name: node.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月26日 星期日 11时41分23秒
 ************************************************************************/

#include "./node.h"

HashFind *init_hash() {
    HashFind *table = (HashFind*) malloc (sizeof(HashFind));
    table->cnt = MAXHASH;
    table->arr = (Node**) malloc (sizeof(Node*) * MAXHASH);
    for (int i = 0; i < MAXHASH; i++) {
        struct sockaddr_in addr;
        table->arr[i] = node_create(addr, -1);
    }
    return table;
}

HashFind *insert_node(HashFind *table, Node *node) {
    int inx = hash_func(node->addr); 
    pthread_mutex_lock(&heart_mutex);
    Node *p = table->arr[inx];
    while (p->next) {
        p = p->next;
    }
    p->next = node;
    pthread_mutex_unlock(&heart_mutex);
    return table;
}

HashFind *delete_node(HashFind *table, struct sockaddr_in addr) {
    int inx = hash_func(addr);
    pthread_mutex_lock(&heart_mutex);
    Node *p, *q;
    p = table->arr[inx];
    while (p->next && (p->next->addr.sin_addr.s_addr != addr.sin_addr.s_addr)) {
        p = p->next;
    }
    if (p->next) {
        q = p->next;
        p->next = q->next;
        free(q);
    }
    pthread_mutex_unlock(&heart_mutex);
    return table;
}


void clear(HashFind *table) {
    for (int i = 0; i < table->cnt; i++) {
        Node *p = table->arr[i];
        while (p != NULL) {
            Node *q = p;
            p = p->next;
            free(q);
        }
    }
    return ;
}

void output(HashFind *table) {
    pthread_mutex_lock(&heart_mutex);
    printf("client资源列表:\n");
    for (int i = 0; i < table->cnt; i++) {
        Node *p = table->arr[i];
        while (p->next) {
            printf("%s fd[%d]\n", inet_ntoa(p->next->addr.sin_addr), p->next->fd);
            p = p->next;
        }
    }
    pthread_mutex_unlock(&heart_mutex);
    return ;
}

int look_ip(HashFind *table, struct sockaddr_in addr) {
    int inx = hash_func(addr);
    pthread_mutex_lock(&heart_mutex);
    LinkList p = table->arr[inx];
    while (p->next) {
        if (p->next->addr.sin_addr.s_addr == addr.sin_addr.s_addr) {
            pthread_mutex_unlock(&heart_mutex);
            return 1;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&heart_mutex);
    return 0;
}

Node *node_create(struct sockaddr_in addr, int fd) {
    Node *p = (Node*) malloc(sizeof(Node));
    p->fd = fd;
    p->cnt = 0;
    p->addr = addr;
    p->next = NULL;
    return p;
}

int hash_func(struct sockaddr_in addr) {
    unsigned int ip = ntohl(addr.sin_addr.s_addr);
    return  ip % MAXHASH;
}


