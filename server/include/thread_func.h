#ifndef _THREAD_FUNC_
#define  _THREAD_FUNC_
#include "factory.h"
int recvn(int sfd,void *pstart,int len);
void * nucleus_func(void *p);
int cmd_handle(char *cmd,char *path);
int tran_file(int new_fd,char *filename);
#endif
