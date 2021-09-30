#include <iostream> 
#include <stdio.h> 
#include <sys/types.h>  
#include <sys/epoll.h>
#include <sys/socket.h> 
#include <sys/time.h>  
#include <sys/un.h>   
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdio>
#include <fcntl.h>
#include <ctime>
#include "list.hpp"
#include "CAN.hpp"
#include "libcuckoo/cuckoohash_map.hh"

#define MAXSIZE 1024*32
#define MESGSIZE 64
#define APP_SERVICE "APP_SERVICE" //对APP的socket fd
#define CAN_SERVICE "CAN_SERVICE" //对CAN的socket fd
#define TIME_TEST 0

/* 创建一个长度为MAXSIZE的safeList */
List<CANRequest> dList(MAXSIZE);

/* 创建一个哈希表 */
libcuckoo::cuckoohash_map<int,int> Table;

int accept_fd;

/* MidLayer作为APPClient的服务器的线程函数 */
void *thrd_serv_func(void *arg)
{
    /* 中间层作为APP服务器,创建socket */
    int server_fd;//监听fd
    server_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(server_fd < 0)
    {
        std::cout<<"MidLayer server socket ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }    
    int flag = fcntl(server_fd, F_GETFL, 0);  // 将监听fd改为非阻塞模式  
    fcntl(server_fd,F_SETFL,flag | O_NONBLOCK);

    /* 设置server_fd的socket选项避免状态在套接字关闭后约保留 2 到 4 分钟  */
    unsigned int on = 1;  
    if((setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("MidLayer setsockopt failed");  
        exit(1);
    } 
 
    /* 填充自身作APP服务器的地址参数 */
    struct sockaddr_un srv_addr;
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path,APP_SERVICE,sizeof(srv_addr.sun_path)-1); 

    /* 作为服务器,绑定socket地址 */
    int ret = 0;
    if((ret = bind(server_fd,(struct sockaddr *)&srv_addr,sizeof(srv_addr))) < 0)
    {
        std::cout<<"MidLayer as server bind ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        close(server_fd);
        unlink(APP_SERVICE);
        exit(1);
    }

    /* 作为服务器,监听自身 */
    if((ret = listen(server_fd,5)) < 0)
    {
        std::cout<<"MidLayer as server listen ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        close(server_fd);
        unlink(APP_SERVICE);
        exit(1);
    }

    /* --------------------------- 作为服务器的epoll -------------------------- */
   
    int server_epfd;//epoll红黑树描述符
    struct epoll_event server_ev;
    struct epoll_event server_events[MAXSIZE];//ev用于注册事件,events用于回传要处理的事件
    /* 1.epoll_create()创建epoll红黑树描述符 */
    if((server_epfd = epoll_create(MAXSIZE)) < 0)
    {
        std::cout<<"MidLayer epoll_create ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }
    /* 2.epoll_ctl()将关心的文件描述符,添加到红黑树事件集中 */
    server_ev.events =  EPOLLIN | EPOLLET;//监听的描述符处于可读状态;将epoll event通知模式设置成边缘触发
    server_ev.data.fd = server_fd;//设置要处理的事件相关的描述符
    /* 将server_fd增添到事件集中 */
    if(epoll_ctl(server_epfd,EPOLL_CTL_ADD,server_fd,&server_ev) < 0)
    {
        std::cout<<"MidLayer server epoll_ctl server_fd ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }
    printf("\n--------------MidLayer work as server, waitting for AppClient's request--------------\n");

    /* 接收AppClient连接 */
    int event;
    int nfds;
    int accept_fd;
    int i = 0;
    char server_msg[MESGSIZE];
    int server_MesgRecvSize = 0;
    int epoll_recv_fd = server_events[i].data.fd;
    while(1)
    {
        /* 3.epoll_wait()阻塞等待注册事件发生,返回事件数目,并将触发的事件写入events数组中 */
        if((nfds = epoll_wait(server_epfd,server_events,MAXSIZE,-1)) > 0)/* 处理返回的事件 */
        {
             for (i = 0; i < nfds; ++i)
             {
                server_MesgRecvSize = 0;
                epoll_recv_fd = server_events[i].data.fd;
                /* Midlayer作为服务器,收到有AppClient连接请求 */
                if(epoll_recv_fd == server_fd)
                {
                    if((accept_fd = accept(server_fd,(struct sockaddr*)NULL,NULL)) < 0)
                    {
                        printf("MidLayer server accept ERROR: %s  errno: %d\n",strerror(errno),errno);
                    }else
                    {
                        printf("AppClient %d connect!\n",accept_fd);
                    }
                    server_ev.events = EPOLLIN;
                    server_ev.data.fd = accept_fd;

                    flag = fcntl(accept_fd, F_GETFL, 0);          // Get socket flags  
                    fcntl(accept_fd,F_SETFL,flag | O_NONBLOCK);   // Add non-blocking flag 

                    if(epoll_ctl(server_epfd,EPOLL_CTL_ADD,accept_fd,&server_ev) < 0)//将accept_fd添加到事件集中
                    {
                        std::cout<<"MidLayer server epoll_ctl add server accept_fd ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
                        exit(1);
                    }
                }   
                else 
                {
                   //处理客户端发送的数据   
                    memset(server_msg,0,sizeof(server_msg));                 
                    if((server_MesgRecvSize = read(epoll_recv_fd,server_msg,MESGSIZE)) > 0)//读取客户端发送的数据成功
                    {                    
                        event = atoi(server_msg);
                        Table.insert(event,accept_fd);  
                    }else if(server_MesgRecvSize == 0)//read返回0读到文件尾,说明对方已经断开连接
                    {
                        printf("Appclient has disconnected!\n");
                        epoll_ctl(server_fd,EPOLL_CTL_DEL,server_fd,NULL);//将server_fd从事件集中删除
                        close(epoll_recv_fd);
                        continue;
                    }else
                    {
                        std::cout<<"MidLayer server read ERROR: "<<strerror(errno)<<"errno: "<<errno<<std::endl;
                        continue;
                    }                
                }  
             }  
        }else if (nfds == 0)
        {
            std::cout<<"time out, No data!\n"<<std::endl;
        }
        /* timeout=-1,直到有fd进入ready状态或者捕获到信号才返回 */
        else
        {
            std::cout<<"MidLayer server epoll_wait ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
            exit(1);
        }       
    }
    close(server_fd);
    close(server_epfd);
    pthread_exit(NULL);
}


/* MidLayer作为CANServer的客户端的线程函数 */
void *thrd_clit_func(void *arg)
{ 
    /* 中间层作为CAN客户端,创建socket */
    int client_fd;//监听fd
    client_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(client_fd < 0)
    {
        std::cout<<"MidLayer client socket ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }  
    auto flag = fcntl(client_fd, F_GETFL, 0);  // 将fd改为非阻塞模式  
    fcntl(client_fd,F_SETFL,flag | O_NONBLOCK);

    /* 填充自身作CANServer客户端的地址参数 */
    struct sockaddr_un cli_addr;
    cli_addr.sun_family = AF_UNIX;
    strncpy(cli_addr.sun_path,CAN_SERVICE,sizeof(cli_addr.sun_path)-1);

    /* 作为客户端,connect连接到CANServer */ 
    int ret = 0; 
    if((ret = connect(client_fd,(struct sockaddr *)&cli_addr,sizeof(cli_addr))) < 0)
    {
        std::cout<<"MidLayer as client connect ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        close(client_fd);
        exit(1);
    }

    /* --------------------------- 作为客户端的epoll -------------------------- */ 
    int client_epfd;//epoll红黑树描述符
    struct epoll_event client_ev;
    struct epoll_event client_events[MAXSIZE];//ev用于注册事件,events用于回传要处理的事件
    /* 1.epoll_create()创建epoll红黑树描述符 */
    if((client_epfd = epoll_create(MAXSIZE)) < 0)
    {
        std::cout<<"MidLayer client epoll_create ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }
    
    /* 2.epoll_ctl()将关心的文件描述符,添加到红黑树事件集中 */
    client_ev.events = EPOLLIN ;//监听的描述符处于可读写状态;将epoll event通知模式设置成边缘触发
    client_ev.data.fd = client_fd;//设置要处理的事件相关的描述符
    /* 将client_fd增添到事件集中 */
    if(epoll_ctl(client_epfd,EPOLL_CTL_ADD,client_fd,&client_ev) < 0)
    {
        std::cout<<"MidLayer client epoll_ctl client_fd ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }
    printf("\n--------------MidLayer work as client--------------\n");

    /* 连接CANServer */
    int nfds;
    int i;
    int epoll_recv_fd;
    int client_MesgRecvSize;
    char client_msg[MESGSIZE];
    uint32_t msgTmp_event;
    uint32_t msgTmp_fd;           
    char chBuffer[MESGSIZE];
    char *pchTmp = NULL;
    char *tmpSaveMsg = NULL;
    CANRequest *pcan_msg;
    CANRequest can_msg;
    struct timeval caltv;
    struct timeval cantv;
    while(1)
    {
        /* 3.epoll_wait()阻塞等待注册事件发生,返回事件数目,并将触发的事件写入events数组中 */
        if((nfds = epoll_wait(client_epfd,client_events,MAXSIZE,-1)) > 0)/* 处理返回的事件 */
        {
            for (i = 0; i < nfds; ++i)
            {      
                epoll_recv_fd = client_events[i].data.fd;
                /* Midlayer作为客户端,连接服务器 */             
                if (epoll_recv_fd == client_fd)//若是发来数据请求,则读取socket并处理
                {                   
                    /* 读取CANServer发送的数据 */
                    if((client_MesgRecvSize = read(epoll_recv_fd,client_msg,64)) > 0)
                    {
                        /* 处理CANServer服务器发送的数据,将其插入list中 */
                        pcan_msg = (CANRequest*)client_msg;
                        can_msg = *pcan_msg;
                        #if TIME_TEST
                        gettimeofday(&caltv,NULL);                                        
                        printf("read  %d  %ld ",can_msg.event,(caltv.tv_sec*1000000 + caltv.tv_usec));
                        #endif
                        dList.addtail(can_msg);
                        #if TIME_TEST
                        gettimeofday(&cantv,NULL);                                        
                        printf("aL  %d  %ld ",can_msg.event,(cantv.tv_sec*1000000 + cantv.tv_usec));
                        #endif
                    }
                    else if(client_MesgRecvSize == 0)//read返回0读到文件尾,说明对方已经断开连接
                    {
                        std::cout<<"CANServer has disconnected!"<<std::endl;
                        epoll_ctl(client_epfd,EPOLL_CTL_DEL,client_fd,NULL);//将client_fd从事件集中删除
                        close(client_fd);
                        continue;
                    }                  
                    else
                    {
                        std::cout<<"MidLayer client read ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
                        continue;
                    }
                }
            }
        }else if (nfds == 0)
        {
            std::cout<<"time out, No data!\n"<<std::endl;
        }
        /* timeout=-1,直到有fd进入ready状态或者捕获到信号才返回 */
        else
        {
            std::cout<<"MidLayer client epoll_wait ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
            exit(1);
        }
    }
    close(client_fd);
    close(client_epfd);
    pthread_exit(NULL);
}


/* MidLayer为服务器线程与客户端线程间进行数据通信的线程函数 */
void *thrd_comm_func(void *arg)
{
    printf("------------thrd_comm_func----------------");
    CANRequest CANrq;
    char commMsg[MESGSIZE];   
    int transFd;
    char input[5];
    int flag;
    struct timeval bfpoptv;
    struct timeval afpoptv;
    fgets(input, 4, stdin);
    if((strcmp(input,"pop")==0))
    {
        printf("----------------pop list----------------\n");
        memset(input,0,sizeof(input));
        while(1)
        {
            CANrq = dList.pop();   
            #if TIME_TEST
            gettimeofday(&bfpoptv,NULL);                                        
            printf("bf find  %d  %ld",CANrq.event,(bfpoptv.tv_sec*1000000 + bfpoptv.tv_usec));      
            #endif
            if (Table.find(CANrq.event,transFd)) 
            {
                #if TIME_TEST
                gettimeofday(&afpoptv,NULL);                                        
                printf(" af find  %d  %ld\n",CANrq.event,(afpoptv.tv_sec*1000000 + afpoptv.tv_usec));
                #endif
                flag = fcntl(transFd, F_GETFL, 0);   
                fcntl(transFd,F_SETFL,flag | O_NONBLOCK);   
                if(send(transFd,&(CANrq.event),MESGSIZE,0) < 0)
                {
                    std::cout<<"MidLayer thrd_comm transfer msg to Appclient write ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
                }   
            }          
        }
    }
   pthread_exit(NULL);
}


int main(int argc, const char** argv) {

    int ret;
    unlink(APP_SERVICE);//解除服务器server_fd绑定的文件

    pthread_t serv_thread;
    pthread_t clit_thread;
    pthread_t comm_thread;
    
    /* 创建服务器线程 */
    ret = pthread_create(&serv_thread,NULL,thrd_serv_func,NULL);
    usleep(1000);
    if(ret < 0)
    {
        std::cout<<"pthread_create serv_thread ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(ret);
    }
    std::cout<<"pthread_create serv_thread success!"<<std::endl;

    /* 创建客户端线程 */
    ret = pthread_create(&clit_thread,NULL,thrd_clit_func,NULL);
    usleep(1000);
    if(ret < 0)
    {
        std::cout<<"pthread_create clit_thread ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(ret);
    }
    std::cout<<"pthread_create clit_thread success!"<<std::endl;

    /* 创建数据通信线程 */
    ret = pthread_create(&comm_thread,NULL,thrd_comm_func,NULL);
    usleep(1000);
    if(ret < 0)
    {
        std::cout<<"pthread_create comm_thread ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(ret);
    }
    std::cout<<"pthread_create comm_thread success!"<<std::endl;


    /* 线程回收 */
    ret = pthread_join(serv_thread,NULL);
    if(ret < 0)
    {
        std::cout<<"pthread_join serv_thread ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(ret);
    }
    std::cout<<"pthread_join serv_thread success!"<<std::endl;

    ret = pthread_join(clit_thread,NULL);
    if(ret < 0)
    {
        std::cout<<"pthread_join clit_thread ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(ret);
    }
    std::cout<<"pthread_join clit_thread success!"<<std::endl;

    ret = pthread_join(comm_thread,NULL);
    if(ret < 0)
    {
        std::cout<<"pthread_join comm_thread ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(ret);
    }
    std::cout<<"\n pthread_join comm_thread success!"<<std::endl;
    return 0;
}