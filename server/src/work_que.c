#include"work_que.h"

int que_init(pque_t pq,int capacity)
{
    bzero(pq,sizeof(que_t));
    pq->que_capacity=capacity;
    pthread_mutex_init(&pq->mutex,NULL);
    return 0;
}
int que_set(pque_t pq,pnode_t pnew)
{
    if(pq->que_size==0)
    {
        pq->que_tail=pq->que_head=pnew;
    }else{
        pq->que_tail->pnext=pnew;
        pq->que_tail=pnew;
    }
    pq->que_size++;
    return 0;
}
int que_get(pque_t pq,pnode_t *ppcur)
{
    if(pq->que_size==0)
    {
        return -1;
    }
    *ppcur=pq->que_head;
    pq->que_head=pq->que_head->pnext;
    pq->que_size--;
    return 0;
  }
