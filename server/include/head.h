#ifndef _HEAD_
#define _HEAD_
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h> 
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h> //addrr.in
#include<arpa/inet.h>//inet_addr
#include<dirent.h>//oepndir
#include<sys/stat.h>
#include<fcntl.h>
#include<grp.h>
#include<pwd.h>
#include<shadow.h>
#include<crypt.h>
#include<sys/mman.h>//mmap
#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcname) {if(retval==ret){perror(funcname);return -1;}}

typedef struct{
    int data_len;//存储真正车厢上有多少数据
    char buf[1000];//装数据的车厢        
}train_t;


#endif 
