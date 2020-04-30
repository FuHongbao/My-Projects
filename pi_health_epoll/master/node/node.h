/*************************************************************************
	> File Name: node.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2020年04月26日 星期日 11时28分49秒
 ************************************************************************/

#ifndef _NODE_H
#define _NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAXHASH 1007
typedef struct Node {
    int fd;
    int cnt;
    struct sockaddr_in addr;
    struct Node *next;
} Node, *LinkList;

pthread_mutex_t heart_mutex;


typedef struct HashFind {
    Node **arr;
    int cnt;
} HashFind;

HashFind *init_hash();
int hash_func(struct sockaddr_in addr);
HashFind *insert_node(HashFind *table, Node *node);
HashFind *delete_node(HashFind *table, struct sockaddr_in addr);
void clear(HashFind *table);
void output(HashFind *table);
int look_ip(HashFind *table, struct sockaddr_in addr);
Node *node_create(struct sockaddr_in addr, int fd);

#endif
