#include <iostream> 
#include <stdio.h> 
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/time.h> 
#include <sys/un.h>   
#include <cstdio> 
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include "CAN.hpp"
 
#define APP_SERVICE "APP_SERVICE" 
#define MAXSIZE 1024*32
#define MESGSIZE 64
#define TIME_TEST 1

int main(int argc, const char** argv) 
{  
    /* 创建socket */
    int socket_fd;//监听fd
    socket_fd = socket(AF_UNIX,SOCK_STREAM,0);//将socket设为非阻塞模式 
    if(socket_fd < 0)
    {
        std::cout<<"AppClient socket ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        return 1;
    } 

    /* 配置客户端参数 */
    struct sockaddr_un cli_addr;
    cli_addr.sun_family = AF_UNIX;
    strcpy(cli_addr.sun_path,APP_SERVICE);

    /* connect连接到服务器 */
    int ret;  
    if((ret = connect(socket_fd,(struct sockaddr *)&cli_addr,sizeof(cli_addr))) < 0)
    {
        std::cout<<"AppClient connect ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        close(socket_fd);
        exit(1);
    }
    std::cout<<"--------------AppClient connect to MidLayer, sending data-------------------"<<std::endl;
    
    auto flag = fcntl(socket_fd, F_GETFL, 0);          // Get socket flags  
    fcntl(socket_fd,F_SETFL,flag | O_NONBLOCK);   // Add non-blocking flag 

    /* --------------------------- 作为客户端的epoll -------------------------- */ 
    int epfd;//epoll红黑树描述符
    struct epoll_event ev;
    struct epoll_event events[MAXSIZE];//ev用于注册事件,events用于回传要处理的事件
    /* 1.epoll_create()创建epoll红黑树描述符 */
    if((epfd = epoll_create(MAXSIZE)) < 0)
    {
        std::cout<<"AppClient epoll_create ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }
    
    /* 2.epoll_ctl()将关心的文件描述符,添加到红黑树事件集中 */
    ev.events =  EPOLLIN ;//监听的描述符处于可读写状态;将epoll event通知模式设置成边缘触发
    ev.data.fd = socket_fd;//设置要处理的事件相关的描述符
    /* 将socket_fd增添到事件集中 */
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,socket_fd,&ev) < 0)
    {
        std::cout<<"Appclient epoll_ctl socket_fd ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        exit(1);
    }

    int i;
    char sendbuf[MESGSIZE];
    for(i = 1;i < 500; ++i)
    {
        memset(sendbuf,0,MESGSIZE);
        sprintf(sendbuf,"%d",i);
        if(send(socket_fd, sendbuf, MESGSIZE,0) < 0)
        {
            std::cout<<"Appclient send ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
            return 1;
        } 
        printf("Appclient will send: %d\n",i);
    }
    
    /* 数据交互读写 */
    int nfds;
    int epoll_recv_fd;
    int client_MesgRecvSize;
    char client_msg[MESGSIZE];
    
    CANRequest CANrq;
    struct timeval tv,caltv;

    char chBuffer[MESGSIZE];
    char *pchTmp = NULL;
    char *tmpSaveMsg = NULL;
    while(1)
    {
        /* 3.epoll_wait()阻塞等待注册事件发生,返回事件数目,并将触发的事件写入events数组中 */
        if((nfds = epoll_wait(epfd,events,MAXSIZE,-1)) > 0)/* 处理返回的事件 */
        {
            for (i = 0; i < nfds; ++i)
            {    
                /* Midlayer作为客户端,连接服务器 */             
                if (events[i].events & EPOLLIN)//若是发来数据请求,则读取socket并处理
                {              
                    /* 读取MidLayer发送的数据 */
                    if((client_MesgRecvSize = recv(socket_fd, &CANrq, MESGSIZE,0)) > 0)
                    {
                        #if TIME_TEST
                        gettimeofday(&caltv,NULL);
                        printf("%d   %ld\n",CANrq.event,(caltv.tv_sec*1000000 + caltv.tv_usec));
                        #endif
                    }
                    else if(client_MesgRecvSize < 0)//read失败
                    {
                        std::cout<<"Appclient read ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
                        continue;
                    }else//read返回0读到文件尾,说明对方已经断开连接
                    {
                        std::cout<<"CANServer has disconnected!"<<std::endl;
                        epoll_ctl(epfd,EPOLL_CTL_DEL,socket_fd,NULL);//将client_fd从事件集中删除
                        close(socket_fd);
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
    close(socket_fd);
    close(epfd);
    pthread_exit(NULL);
}   