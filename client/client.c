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
    printf("please input ures name:");
    char name[20]={0};
    char path[256]={0};
    int data_len=0;
    scanf("%s",name);
    //发送用户名
    train_t t;
    bzero(&t,sizeof(train_t));
    strcpy(t.buf,name);
    t.data_len=strlen(t.buf);
    ret=send(sfd,&t,4+t.data_len,MSG_NOSIGNAL);
    ERROR_CHECK(ret,-1,"send_name");
    print_options();//打印指示
    //接收相对路径
    ret=recvn(sfd,&data_len,4);
    ERROR_CHECK(ret,-1,"recvn");
    ret=recvn(sfd,&path,data_len);
    ERROR_CHECK(ret,-1,"recvn");
    printf("%s\n",path);
    while(1)
    {
        //读取并分离命令 ”操作 对象“
        char cmd[256]={0};
        char op[16]={0};
        char ob[256]={0};
        int ret=read(STDIN_FILENO,cmd,sizeof(cmd));
        ERROR_CHECK(ret,-1,"read");
        split_cmd(cmd,op,ob);
        printf("!!!!!!!!!!\n");
        
        
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
            printf("%d\n",data_len);
            ret=recvn(sfd,path,data_len);
            ERROR_CHECK(ret,-1,"recvn");
            printf("%s\n",path);
        }
        
        if(strcmp(op,"pwd")==0)
        {
            data_len=0;
            bzero(&path,sizeof(path));
            ret=recvn(sfd,&data_len,4);
            ERROR_CHECK(ret,-1,"recvn");
            ret=recvn(sfd,path,data_len);
            ERROR_CHECK(ret,-1,"recvn");
            printf("%s\n",path);
        }
        
        if(strcmp(op,"ls")==0)
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
        if(strcmp(op,"gets")==0)
        {
            char buf[1000]={0};
            int data_len=0;
            off_t file_size=0;
            //接受文件大小
            int ret=recv(sfd,&data_len,4,0);
            if (ret==-1)printf("recv g\n");
            ret=recv(sfd,&file_size,data_len,0);
            if(ret==-1)printf("recv gg\n");
            printf("%ld\n",file_size);
            //创建打开文件
            int fd =open(ob,O_WRONLY|O_CREAT,0666);
            if(fd==-1)printf("open gg\n");
            while(1)
            {
                recv(sfd,&data_len,4,0);
                if(data_len>0)
                {
                    recv(sfd,buf,data_len,0);
                    write(fd,buf,data_len);
                }else{
                    break;
                }
            }
            close(fd);
        }
        if(strcmp("puts",op)==0)
        {
            printf("start upload\n");
            int fd=open(ob,O_RDONLY);
            if(fd==-1)printf("open gg\n");
            //发送文件大小
            struct stat buf;
            int ret=fstat(fd,&buf);
            if(ret==-1)printf("ret gg\n");
            t.data_len=sizeof(buf.st_size);
            memcpy(t.buf,&buf.st_size,t.data_len);
            ret = send (sfd,&t,4+t.data_len,0);
            if(ret==-1)printf("send gg\n");
            printf("befoe while\n");
            //发送文件内容
            while((t.data_len=read(fd,t.buf,sizeof(t.buf))))
            {
                printf("T.DATA-lEN=%d",t.data_len);
                ret=send(sfd,&t,4+t.data_len,0);    
                if(ret==-1)printf("send error\n");
                printf("ret=%d",ret);
            }
            printf("after whiel\n");
            ret=send(sfd,&t,4,0);
            if(ret==-1)printf("send2 error\n");
            printf(" i have send\n");
            close(fd);
        }
    }
    return 0;
}
