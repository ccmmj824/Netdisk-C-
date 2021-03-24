#ifndef _FACTORY_
#define _FACTORY_
#include"head.h"
#include"work_que.h"
typedef void* (*thread_func_t)(void*);
typedef struct{
    que_t que;
    pthread_cond_t cond;
    pthread_t *pthid;
    int pthread_num;
    thread_func_t factory_func;
    int start_flag;
}factory_t;
int factory_init(factory_t* pnuclues,int thread_num,int capacity);
int factory_start(factory_t* pnuclues);
int tcp_init(int *psfd,char *ip,char *prot);
void conf_init(char *path,char *ip,char *port,char*pthread_num,char *capacity);
void arg_set(char *line,char *arg);
#endif

