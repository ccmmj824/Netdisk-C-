#include"head.h"
void print_options()
{
    printf("         1.cd          进入对应目录\n"); 
    printf("         2.ls          列出相应目录文件\n"); 
    printf("         3.puts file   将本地文件上传至服务器\n"); 
    printf("         4.gets file   下载服务器文件到本地\n"); 
    printf("         5.remove file 删除服务器上文件\n"); 
    printf("         6.pwd         显示目前所在路径\n"); 
    printf("         7.其他命令错误，重新输入\n"); 
}
int tran_file(int sfd,char *filename)
{
    train_t t;
    int fd=open(filename,O_RDONLY);
    ERROR_CHECK(fd,-1,"open");
    //发送文件大小
    struct stat buf;
    int ret = fstat(fd,&buf);
    ERROR_CHECK(ret,-1,"fstat");
    t.data_len=sizeof(buf.st_size);
    memcpy(t.buf,&buf.st_size,t.data_len);
    send(sfd,&t,4+t.data_len,0);
    //MMAP零拷贝技术发送文件内容                                    
    char *pstart=(char *)mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
    ERROR_CHECK(pstart,(char*)-1,"mmap");
    printf("buf.st_size=%ld\n",buf.st_size);
    ret=send(sfd,pstart,buf.st_size,MSG_NOSIGNAL);//发送一个空的>
    ERROR_CHECK(ret,-1,"send");
    ret=munmap(pstart,buf.st_size);
    ERROR_CHECK(ret,-1,"munmap");
    return 0;
}
void split_cmd(char *cmd,char *op,char *ob)
{
    int i=0;
    char *p=cmd;
    while(*p!='\n'&&*p!=' ')
    {
        op[i++]=*(p++);
    }
    i=0;
    if(*p!='\n')
    {
        p++;
    }
    while(*p!='\n')
    {
        ob[i++]=*(p++);
    }
}

int recvn(int sfd, void *pstart,int len)
{
    int total=0,ret=0;
    char *p=(char *)pstart;
    while(total<len)
    {
        ret=recv(sfd,p+total,len-total,0);
        if(ret==0)
        {
            printf("\n");
            return -1;
        }
        total+=ret;
    }
    return 0;
}
