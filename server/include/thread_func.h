#ifndef _THREAD_FUNC_
#define  _THREAD_FUNC_
#include "factory.h"
int recvn(int sfd,void *pstart,int len);
void * nucleus_func(void *p);
int cmd_handle(char *cmd,char *path);
int tran_file(int new_fd,char *filename);
int is_user(char *name,char *passed);
void get_salt(char *salt,char *passwd);
void log_client(FILE *log_client,const char *name,int flag);
#endif
