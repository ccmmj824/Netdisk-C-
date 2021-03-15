#ifndef _HEAD_
#define _HEAD_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h> 
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h> //addrr.in
#include<arpa/inet.h>//inet_addr
#include <unistd.h>
#include<fcntl.h>//O_RDWR
#include<dirent.h>
#include<sys/stat.h>
#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcname) {if(retval==ret){perror(funcname);return -1;}}
void print_options();
void split_cmd(char *cmd ,char *op,char *ob);
int recvn(int sfd, void *pstart,int len);

typedef struct{
    int data_len;//存储真正车厢上有多少数据
    char buf[1000];//装数据的车厢                                                             
}train_t;
#endif 
