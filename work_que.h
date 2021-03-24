#ifndef _QUE_
#define _QUE_
#include"head.h"
typedef struct node{
    int new_fd;
//    char usr_path[128];
    char pur_path[256];
    struct node *pnext;
}node_t,*pnode_t;
typedef struct{
    pnode_t que_head,que_tail;
    int que_capacity;
    int que_size;
    pthread_mutex_t mutex;
}que_t,*pque_t;
int que_init(pque_t pq,int capacity);
int que_set(pque_t pq,pnode_t pnew);
int que_get(pque_t pq,pnode_t *ppcur);
#endif
