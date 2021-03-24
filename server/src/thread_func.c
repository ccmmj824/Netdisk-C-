#include"thread_func.h"

//登录登出日志记录
void log_client(FILE* client_fp,const char *name,int flag)
{
    //当前时间
    time_t now;
    struct tm *timenow;
    time(&now);
    timenow=localtime(&now);
    fprintf(client_fp,"%s",asctime(timenow));
    if(flag==1)
    {
        fprintf(client_fp,"---> %s <----logIN!\n\n",name);
    }else{
        fprintf(client_fp,"---> %s <----logOUT!\n\n",name);
    }
}
//用户命令记录 
void log_name(FILE* name_fp,char *op,char *ob)
{
    //当前时间
    time_t now;
    struct tm *timenow;
    time(&now);
    timenow=localtime(&now);
    fprintf(name_fp,"%s",asctime(timenow));
    fprintf(name_fp,"%s %s\n",op,ob);

}

int is_user(char *name,char *passwd)
{
    // 加密格式：
    // $6$salt$passwd 

    struct spwd *sp;
    char salt[512]={0};
    //获取完整密码
    sp=getspnam(name);
    if(NULL==sp)
    {
        perror("getspnam");
        return -1;
    }
    //获取盐值
    get_salt(salt,sp->sp_pwdp);
    if(!strcmp(sp->sp_pwdp,crypt(passwd,salt)))
    {
        printf("登陆成功\n");
        return 0;
    }else{
        printf("密码错误\n");
        return -1;
    }
}

void get_salt(char *salt,char * passwd)
{
    int i,j;
    for(i=0,j=0;passwd[i]&&j!=3;i++)
    {
        if(passwd[i]=='$')
        {
            ++j;
        }
    }
    strncpy(salt,passwd,i-1);
}
int tran_file(int new_fd,char *filename)
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
    send(new_fd,&t,4+t.data_len,0);
    //MMAP零拷贝技术发送文件内容
    char *pstart=(char *)mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
    ERROR_CHECK(pstart,(char*)-1,"mmap");
    ret=send(new_fd,pstart,buf.st_size,MSG_NOSIGNAL);//发送一个空的火车头到客户端
    ERROR_CHECK(ret,-1,"send");
    ret=munmap(pstart,buf.st_size);
    ERROR_CHECK(ret,-1,"munmap");
    return 0;
}
int recvn(int sfd,void*pstart,int len)
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

void * nucleus_func(void *p)
{
    factory_t *pnucleus=(factory_t *)p;
    pnode_t pcur;
    pque_t pq=&pnucleus->que;
    while(1){
        pthread_mutex_lock(&pq->mutex);
        char usr_name[128]={0};
        char passwd[20]={0};
        int data_len =0;
        int flag=-1;
        train_t  t={0,{0}};
        if(!pq->que_size){
            pthread_cond_wait(&pnucleus->cond,&pq->mutex);
        }
        int ret=que_get(pq,&pcur);
        pthread_mutex_unlock(&pq->mutex);
        if(ret==-1) continue;
        printf("Get Client!\n");
        //登录处理
        //维护客户端请求信息
        FILE* client_fp=fopen("../log/log_client","a");
        if(client_fp==NULL) 
        {
            perror("fopen");
            continue;
        }
        do{
            //接收用户名
            bzero(usr_name,sizeof(usr_name));
            recvn(pcur->new_fd,&data_len,4);
            recvn(pcur->new_fd,usr_name,data_len);
            //日志记录登陆时间。
            log_client(client_fp,usr_name,1);
            //接收明文密码
            bzero(passwd,sizeof(passwd));
            recvn(pcur->new_fd,&data_len,4);
            recvn(pcur->new_fd,passwd,data_len);
            //处理密码
            flag=is_user(usr_name,passwd);
            //发送结果
            bzero(&t,sizeof(t));
            t.data_len=sizeof(int);
            memcpy(t.buf,&flag,t.data_len);
            send(pcur->new_fd,&t,4+t.data_len,MSG_NOSIGNAL);
        }while(flag==-1);

        //维护用户路径
        sprintf(pcur->pur_path,"%s%s","/home/cc/item/netdisk/",usr_name);
        bzero(&t,sizeof(train_t));
        strcpy(t.buf,pcur->pur_path);
        t.data_len=strlen(t.buf);
        ret=send(pcur->new_fd,&t,4+t.data_len,MSG_NOSIGNAL);
        chdir(pcur->pur_path);
        //维护用户日志
        char log_path[128];
        sprintf(log_path,"/home/cc/item/old_server/log/log_%s",usr_name);
        FILE *name_fp=fopen(log_path,"a");
        while(1)
        {
            printf("当前路径 =%s\n",pcur->pur_path);
            //接收"操作"和"对象"
            char op[16]={0};
            char ob[256]={0};
            ret=recvn(pcur->new_fd,&data_len,4);
            if(ret==-1)
            {
                printf("recvn接收op失败,客户端断开连接\n");
                return(void*) -1;
            }
            ret=recvn(pcur->new_fd,op,data_len);
            if(ret==-1)
            {
                printf("recvn接受op失败，客户端断开连接\n");
                return (void *)-1;
            }
            ret=recvn(pcur->new_fd,&data_len,4);
            if(ret==-1)
            {
                printf("recvn接收ob失败,客户端断开连接\n");
                return(void*) -1;
            }
            ret=recvn(pcur->new_fd,ob,data_len);
            if(ret==-1)
            {
                printf("recvn接受ob失败，客户端断开连接\n");
                return (void *)-1;
            }
            //log记录并打印命令
            printf("执行：%s %s\n",op,ob);
            log_name(name_fp,op,ob);
            //处理 操作
            if(strcmp("cd",op)==0)
            {
                sprintf(pcur->pur_path,"%s/%s",pcur->pur_path,ob);
                chdir(pcur->pur_path);
                //发送路径
                train_t t;
                bzero(&t,sizeof(train_t));
                strcpy(t.buf,getcwd(NULL,0));
                printf("t.buf=%s\n",t.buf);
                t.data_len=strlen(t.buf);
                printf("t.dta_len=%d",t.data_len);
                ret=send(pcur->new_fd,&t,4+t.data_len,MSG_NOSIGNAL);
                if (ret==-1)
                {
                    printf("发送路径失败\n");
                    return (void*)-1;
                }
            }
            if(strcmp("pwd",op)==0)
            {
                //发送路径
                train_t t;
                bzero(&t,sizeof(train_t));
                strcpy(t.buf,getcwd(NULL,0));
                printf("t.buf=%s\n",t.buf);
                t.data_len=strlen(t.buf);
                ret=send(pcur->new_fd,&t,4+t.data_len,MSG_NOSIGNAL);
                if (ret==-1)
                {
                    printf("发送路径失败\n");
                    return (void*)-1;
                }
            }
            if(strcmp("ls",op)==0)
            {
                train_t t;
                bzero(&t,sizeof(train_t));
                //准备传送目录文件信息
                //打开目录

                DIR *dirp=opendir(pcur->pur_path);
                if(dirp==NULL){
                    printf("error opendir");
                    return (void* )-1;
                }
                struct dirent *pdirent;
                while((pdirent=readdir(dirp))!=0){
                    struct stat statbuf;
                    stat(pdirent->d_name,&statbuf);
                    sprintf(t.buf,"%d %6o %2ld %12ld %-15s %s",
                            pdirent->d_type,
                            statbuf.st_mode,
                            statbuf.st_nlink,
                            statbuf.st_size,
                            pdirent->d_name,
                            ctime(&statbuf.st_mtime)
                           );
                    t.data_len=strlen(t.buf);
                    send(pcur->new_fd,&t,4+t.data_len,MSG_NOSIGNAL);
                }
                closedir(dirp);
                t.data_len=0;
                send(pcur->new_fd,&t.data_len,sizeof(int),0);
            }
            //下载文件到客户端 MMAP
            if(strcmp("gets",op)==0)
            {   
                printf("===>ob<=====\n");
                printf("start download==========>\n");
                int ret=tran_file(pcur->new_fd,ob);
            	if(ret==-1)printf("gg\n");
			}
            //上传文件到此服务器
            //使用MMAP内存映射的零拷贝技术。
            if(strcmp("puts",op)==0)
            {
                //接收文件大小
                int data_len =0;
                off_t file_size=0; 
                int ret=recv(pcur->new_fd,&data_len,4,0); 
                if(ret==-1)printf("recv gg\n"); 
                ret=recv(pcur->new_fd,&file_size,data_len,0); 
                if(ret==-1)printf("recv gg\n"); 
                printf("filesize=%ld\n",file_size);
                //接收文件
                int fd=open(ob,O_RDWR|O_CREAT,0666);
                if(fd==-1)printf("open gg\n");
                ftruncate(fd,file_size);
                void *pstart=mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
                if(pstart==(void*)-1)perror("mmap");
                ret=recvn(pcur->new_fd,pstart,file_size);
                if(ret==-1){
                    printf("recv gg\n");
                }else{
                    printf("100%% \n");
                }
                munmap(pstart,file_size);
                close(fd);
            }
            if(strcmp("remove",op)==0)
            {
                ret=remove(ob);
                if (-1==ret)printf("remvoe gg\n"); 
            }
            strcpy(pcur->pur_path,getcwd(NULL,0));
            if(strcmp("exit",op)==0)
            {
                break;
            }
        }
        //客户端退出程序处理
        //日志记录登出时间，关闭用户日志，命令日志
        log_client(client_fp,usr_name,-1);
        fclose(client_fp);
        fclose(name_fp);
        //恢复到bin路径
        bzero(pcur->pur_path,sizeof(pcur->pur_path));
        strcpy(pcur->pur_path,"/home/cc/item/old_server/bin/");
        ret=chdir(pcur->pur_path);
        if(ret==-1)
        {
            perror("chdir");
        }
        free(pcur);
        pcur=NULL;
        printf("%s\n",getcwd(NULL,0));
    }
    return 0; 
}
