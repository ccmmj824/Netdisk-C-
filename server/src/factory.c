#include"factory.h"
#include"thread_func.h"
//初始化线程池控制体
int factory_init(factory_t* pnucleus,int thread_num,int capacity)
{
    bzero(pnucleus,sizeof(factory_t));
    //设置控制体队列
    que_init(&pnucleus->que,capacity);
    //设置控制体条件变量
    pthread_cond_init(&pnucleus->cond,NULL);    
    //设置控制体线程起始地址数组
    pnucleus->pthid=(pthread_t *)calloc(thread_num,sizeof(pthread_t));
    //设置线程个数
    pnucleus->pthread_num=thread_num;
    //设置控制体函数
    pnucleus->factory_func=nucleus_func;
    //设置控制体启动标志
    pnucleus->start_flag=0;
    return 0;
}
//子线程启动函数()
int factory_start(factory_t* pnucleus)
{
    if(pnucleus->start_flag==0)
    {   
        printf("开始创建子线程中----->\n");
        for(int i=0;i<pnucleus->pthread_num;i++)
        {   
            pthread_create(pnucleus->pthid+i,NULL,pnucleus->factory_func,pnucleus);
            printf("%d号子线程启动成功\n",i+1);
        }
        pnucleus->start_flag=1;
    }
    return 0;
}
//初始化端口IP，绑定sfd
int tcp_init(int *psfd,char *ip,char *port)
{   
    //创建套接字
    *psfd=socket(AF_INET,SOCK_STREAM,0);//IPV4  TCP
    ERROR_CHECK(*psfd,-1,"socket");
    //创建套端口结构体
    struct sockaddr_in ser_addr;
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_port=ntohs(atoi(port));
    ser_addr.sin_addr.s_addr=inet_addr(ip);
    //设置端口重用；
    int reuse =1;
    int ret=setsockopt(*psfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
    ERROR_CHECK(ret,-1,"setsockopt");
    //绑定套接字结构体和套接字
    ret=bind(*psfd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr));
    ERROR_CHECK(ret,-1,"bind");
    listen(*psfd,10);
    return 0;
}
void arg_set(char *line,char *arg)
{
    char* ptr=strchr(line,'=');
    if(ptr==NULL)printf("error strlen\n");
    strcpy(arg,ptr+1);
}
void conf_init(char *path,char *ip,char*port,char *thread_num,char *capacity)
{
    FILE *fp_conf=fopen(path,"r");
    if(fp_conf==NULL)
    {
        printf("error foepn conf\n");
    }
    char line[128];
    bzero(line,128);
    fgets(line,128,fp_conf);
    line[strlen(line)-1]='\0';
    arg_set(line,ip);
    
    bzero(line,128);
    fgets(line,128,fp_conf);
    line[strlen(line)-1]='\0';
    arg_set(line,port);
    
    bzero(line,128);
    fgets(line,128,fp_conf);
    line[strlen(line)-1]='\0';
    arg_set(line,thread_num);
    
    bzero(line,128);
    fgets(line,128,fp_conf);
    line[strlen(line)-1]='\0';
    arg_set(line,capacity);
}
