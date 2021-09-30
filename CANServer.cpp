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
#include <fcntl.h>
#include <chrono>
#include <ctime>
#include "CAN.hpp"
 
#define CAN_SERVICE "CAN_SERVICE" 
#define MAXSIZE 1024*32
#define MESGSIZE 64
#define TIME_TEST 1

int main(int argc, const char** argv) 
{  
    unlink(CAN_SERVICE);//解除服务器socket_fd绑定的文件

    /* 创建socket */
    int socket_fd;//监听fd
    socket_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(socket_fd < 0)
    {
        std::cout<<"CANServer socket ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        return 1;
    } 

    /* 设置server_fd的socket选项避免状态在套接字关闭后约保留 2 到 4 分钟 */ 
    int on = 1;  
    if((setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("CANServer setsockopt failed");  
        return 1;
    } 

    /* 配置服务器参数 */
    static struct sockaddr_un serv_addr; 
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path,CAN_SERVICE,sizeof(serv_addr.sun_path)-1);

    /* 作为服务器,绑定socket地址 */
    int ret = 0;
    if((ret = bind(socket_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0)
    {
        std::cout<<"CANServer bind ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        close(socket_fd);
        unlink(CAN_SERVICE);
        return 1;
    }

    /* 作为服务器,监听自身 */
    if((ret = listen(socket_fd,5)) < 0)
    {
        std::cout<<"CANServer listen ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
        close(socket_fd);
        unlink(CAN_SERVICE);
        return 1;
    }
    std::cout<<"------------waitting for MidLayer's request-----------------------"<<std::endl;
    
    int accept_fd;
    static struct sockaddr_un cli_addr; 
    CANRequest CANrq;
    /* 接受connect请求 */ 
    accept_fd = accept(socket_fd,(struct sockaddr*)NULL,NULL);  
    if(accept_fd < 0)  
    {  
        perror("CANServer cannot accept MidLayer connect request");  
        close(socket_fd);  
        unlink(CAN_SERVICE);  
        return 1;  
    }else
    {
        printf("MidLayer %d connect!\n",accept_fd);
    }  
 
	/* 向MidLayer写入数据 */   
    int i = 0;   
    char CANevent[MESGSIZE/2];    
    char CANdata[MESGSIZE/2]; 
    struct timeval caltv;    
    sleep(10);
    printf("CANServer start sending mesg!\n");
    for(i = 1;i < 500;++i)
    {
        usleep(40); 
        CANrq.event = i;
        CANrq.data = i + 1;
            
        if(write(accept_fd,&CANrq,MESGSIZE) > 0)
        {
            #if TIME_TEST
            gettimeofday(&caltv,NULL);                                        
            printf("%d  %ld\n",CANrq.event,(caltv.tv_sec*1000000 + caltv.tv_usec));
            #endif
        }else
        {
            std::cout<<"CANServer write ERROR: "<<strerror(errno)<<" errno: "<<errno<<std::endl;
            return 1;
        }               
    }
    sleep(2);
    // 关闭socket
    close(socket_fd);
    unlink(CAN_SERVICE);  
    return 0;
}