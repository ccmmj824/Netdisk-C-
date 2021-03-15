#include"factory.h"
int main(int argc,char **argv)
{
    ARGS_CHECK(argc,2);//.main ip prot pthread_num capacity
    //初始化线程池控制体
    char ip[100]={0};
    char prot[16]={0};
    char pthread_num[16]={0};
    char capa[16]={0};
    conf_init(argv[1],ip,prot,pthread_num,capa);
    int thread_num =atoi(pthread_num);
    int capacity=atoi(capa);
    factory_t nucleus;
    factory_init(&nucleus,thread_num,capacity);
    //启动线程池运行
    factory_start(&nucleus);
    //开启主进程端口监听
    int sfd;
    tcp_init(&sfd,ip,prot);
    //操纵队列 初始化接收缓冲，新队列节点指针，队列指针
    int new_fd;
    pnode_t pnew;
    pque_t pq=&nucleus.que;
    //循环监听端口
    while(1){
        new_fd=accept(sfd,NULL,NULL);
    //收到请求后把此缓冲区描述符发给队列
        pnew=(pnode_t)calloc(1,sizeof(node_t));
        pnew->new_fd=new_fd;
        pthread_mutex_lock(&pq->mutex);
        que_set(pq,pnew);
        pthread_mutex_unlock(&pq->mutex);
    //之后唤醒一个线程
        pthread_cond_signal(&nucleus.cond);
    }
    return 0;
}

