#include "head.h"
int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);
    //初始化一个socket描述符，用于tcp通信
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(sfd,-1,"socket");
    struct sockaddr_in ser_addr;
    bzero(&ser_addr,sizeof(ser_addr));
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_port=htons(atoi(argv[2]));//端口转为网络字节序
    ser_addr.sin_addr.s_addr=inet_addr(argv[1]);
    int ret=connect(sfd,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
    ERROR_CHECK(ret,-1,"connect");
    printf("================= 连接到服务器 ================\n");
    printf("================= IP:%s ============\n",argv[1]);
    printf("================= 端口号:%s =================\n",argv[2]);
    int flag=-1;
    char name[20]={0};
    char *passwd;
    char path[256]={0};
    int data_len=0;
    train_t t;
    do{
        printf("please input ures name:");
        scanf("%s",name);
        passwd=getpass("passwd:");
        //发送用户名
        bzero(&t,sizeof(train_t));
        strcpy(t.buf,name);
        t.data_len=strlen(t.buf);
        ret=send(sfd,&t,4+t.data_len,MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send_name");
        //发送密码
        bzero(&t,sizeof(train_t));
        strncpy(t.buf,passwd,strlen(passwd));
        t.data_len=strlen(t.buf);
        ret=send(sfd,&t,4+t.data_len,MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send_passwd");
        //接收flag
        recv(sfd,&data_len,4,0);
        recv(sfd,&flag,data_len,0);
        if(flag==-1)
        {
            printf("ERROR!input again\n");
        }
    }while(flag==-1);
    printf("登录成功：\n");
    printf("当前路径：");
    //接收绝对路径
    ret=recvn(sfd,&data_len,4);
    ERROR_CHECK(ret,-1,"recvn");
    ret=recvn(sfd,&path,data_len);
    ERROR_CHECK(ret,-1,"recvn");
    puts(path);
    print_options();//打印指示
    
    while(1)
    {
        //读取并分离命令 ”操作 对象“
        char cmd[256]={0};
        char op[16]={0};
        char ob[256]={0};
        printf("----------><----------\n");
        printf("%s\n",path);
        printf("----------><----------\n");
        int ret=read(STDIN_FILENO,cmd,sizeof(cmd));
        ERROR_CHECK(ret,-1,"read");
        split_cmd(cmd,op,ob);

        //发送“操作”
        train_t t;
        bzero(&t,sizeof(train_t));
        strcpy(t.buf,op);
        t.data_len=strlen(t.buf);
        ret=send(sfd,&t,4+t.data_len,MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send_op");
        //发送“对象”
        bzero(&t,sizeof(train_t));
        strcpy(t.buf,ob);
        t.data_len=strlen(t.buf);
        ret=send(sfd,&t,4+t.data_len,MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send_ob");

        if(strcmp(op,"cd")==0)
        {
            data_len=0;
            bzero(&path,sizeof(path));
            ret=recvn(sfd,&data_len,4);
            ERROR_CHECK(ret,-1,"recvn");
            ret=recvn(sfd,path,data_len);
            ERROR_CHECK(ret,-1,"recvn");
        }

        else if(strcmp(op,"pwd")==0)
        {
            data_len=0;
            bzero(&path,sizeof(path));
            ret=recvn(sfd,&data_len,4);
            ERROR_CHECK(ret,-1,"recvn");
            ret=recvn(sfd,path,data_len);
            ERROR_CHECK(ret,-1,"recvn");
            printf("%s\n",path);
        }

        else if(strcmp(op,"ls")==0)
        {
            data_len=0;
            char buf[1014]={0};
            while(1)
            {
                recv(sfd,&data_len,sizeof(int),0);
                if(data_len==0)
                {
                    break;
                }
                recvn(sfd,buf,data_len);
                printf("%s\n",buf);
            }
        }
        //////////////////////
        //////下载文件///////
        else if(strcmp(op,"gets")==0)
        {
            int data_len=0;
            off_t file_size=0;
            int fd=open(ob,O_RDWR|O_CREAT,0666);
            //接受文件大小
            ERROR_CHECK(fd,-1,"open");  
            printf("%s %s",op,ob);
            int ret=recv(sfd,&data_len,4,0);
            if (ret==-1) return -1;
            ret=recv(sfd,&file_size,data_len,0);
            if(ret==-1) return -1;
            printf("file_size=%ld\n",file_size);
            //接收文件内容
            time_t start,end;
            start=time(NULL);
            ftruncate(fd,file_size);
            void *pstart=mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
            ERROR_CHECK(pstart,(void*)-1,"mmap");
            ret=recvn(sfd,pstart,file_size);
            if(ret==-1){
                printf("recv gg\n");
            }else{
                end=time(NULL);
                printf("100%% use time =%ld\n",end-start);
            }
            munmap(pstart,file_size);
            close(fd);
        }
        ///////////上传文件/////////////
        else if(strcmp("puts",op)==0)
        {
            printf("start upload\n");
            printf("path = %s\n",getcwd(NULL,0));
            int ret=tran_file(sfd,ob);
            if(ret==-1)printf("tran_file gg\n");
            printf("Send %s Success\n",ob);
        }else if(strcmp(op,"remove")==0){
            printf("rm %s Success",ob);
        }
        else if(strcmp(op,"exit")==0)
        {
            break;
        }
        else{
            printf("ERROR CMD，请重新输入\n");
        }
    }
    close(sfd);
    return 0;
}
