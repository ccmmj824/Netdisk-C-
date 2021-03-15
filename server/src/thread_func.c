#include"thread_func.h"

void * nucleus_func(void *p)
{
    factory_t *pnucleus=(factory_t *)p;
    pnode_t pcur;
    pque_t pq=&pnucleus->que;
    while(1){
        pthread_mutex_lock(&pq->mutex);
        if(!pq->que_size){
            pthread_cond_wait(&pnucleus->cond,&pq->mutex);
        }
        int ret=que_get(pq,&pcur);
        pthread_mutex_unlock(&pq->mutex);
        if(ret==-1) continue;
        printf(" I HAVE GET CLIENT\n");
        //接收用户名并发送路径
        int data_len=0;
        char usr_name[128]={0};
        ret=recvn(pcur->new_fd,&data_len,4);
        if(ret==-1)
        {
            printf("接收用户名失败,客户端断开连接\n");
            return 0;
        }
        ret=recvn(pcur->new_fd,usr_name,data_len);
        if(ret==-1)
        {
            printf("接受用户名失败，客户端断开连接\n");
            return 0;
        }
        //发送路径
        sprintf(pcur->pur_path,"%s%s","/home/cc/item/netdisk/",usr_name);
        train_t t;
        bzero(&t,sizeof(train_t));
        strcpy(t.buf,pcur->pur_path);
        t.data_len=strlen(t.buf);
        ret=send(pcur->new_fd,&t,4+t.data_len,MSG_NOSIGNAL);
        printf("%s\n",pcur->pur_path);
        if (ret==-1)
        {
            printf("发送路径失败\n");
            return (void*)-1;
        }
        chdir(pcur->pur_path);
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
            printf("执行：%s %s\n",op,ob);
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
            /////////////////////////////////////////
            //////////////下载文件///////////////////
            if(strcmp("gets",op)==0)
            {   
                printf("===>ob<=====\n");
                printf("start download==========>\n");
                train_t t;
                //ret=tran_file(pcur->new_fd,ob);    
                //发送文件大小
                int fd=open(ob,O_RDONLY);
                if(fd==-1)printf("open gg\n");
                struct stat buf;
                int ret=fstat(fd,&buf);
                if(ret==-1)printf("fstat gg\n");
                t.data_len=sizeof(buf.st_size);
                memcpy(t.buf,&buf.st_size,t.data_len);
                ret=send(pcur->new_fd,&t,4+t.data_len,0);
                if(ret==-1)printf("send gg\n"); 
                //ERROR_CHECK(ret,(ssize_t)-1,"send");
                //发送文件内容
                while((t.data_len=read(fd,t.buf,sizeof(t.buf)))) 
                {
                    send(pcur->new_fd,&t,4+t.data_len,0);
                }
                send(pcur->new_fd,&t,4,0);
                printf("end download\n");
                close(fd);
            }
            //上传文件
            if(strcmp("puts",op)==0)
            {
                puts(op);
                puts(ob);
                int data_len;
                char buf[1000];
                //接收文件大小
                off_t file_size;
                int ret=recv(pcur->new_fd,&t.data_len,4,0);
                if(ret==-1)printf("recv gg\n");
                ret=recv(pcur->new_fd,&file_size,t.data_len,0);
                if(ret==-1)printf("recv gg\n");
                printf("filesize=%ld",file_size);
                //创建打开文件
                int fd=open(ob,O_WRONLY|O_CREAT,0666);
                if(fd==-1)printf("open gg\n");
                while(1)
                {
                    recv(pcur->new_fd,&data_len,4,0);
                    if(data_len>0)
                    {
                        recv(pcur->new_fd,buf,data_len,0);
                        write(fd,buf,data_len);
                    }else{
                        break;
                    }
                }
                close(fd);
            }
            if(strcmp("remove",op)==0)
            {
                ret=remove(ob);
                if (-1==ret)printf("remvoe gg\n"); 
            }
            strcpy(pcur->pur_path,getcwd(NULL,0));
        }
    }
    return 0; 
}

int tran_file(int new_fd,char filename[])
{
    train_t t;
    t.data_len=strlen(filename);
    printf("name_len =%d",t.data_len);
    send(new_fd,&t,4+t.data_len,0);
    int fd=open(filename,O_RDONLY);
    ERROR_CHECK(fd,-1,"open");
    //发送文件大小
    struct stat buf;
    int ret = fstat(fd,&buf);
    ERROR_CHECK(ret,-1,"fstat");
    t.data_len=sizeof(buf.st_size);
    memcpy(t.buf,&buf.st_size,t.data_len);
    send(new_fd,&t,4+t.data_len,0);
    while((t.data_len=read(fd,t.buf,sizeof(t.buf))))
    {
        send(new_fd,&t,4+t.data_len,0);
    }
    send (new_fd,&t,4,0);
    printf("can y see me \n");
    return 0;
}
int recvn(int sfd,void*pstart,int len)
{
    int total=0,ret;
    char *p=(char *)pstart;
    while(total<len)
    {   
        ret=recv(sfd,p+total,len-total,MSG_WAITALL);
        if(ret==0)
        {   
            printf("\n");
            return -1; 
        }   
        total+=ret;
    }   
    return 0;
}

