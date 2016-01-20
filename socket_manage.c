#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "config_manage.h"
#include "socket_manage.h"

static int socksed,sockrecv;   //发送接收数据的套接字描述符
static int sendPort = 3000;       //目标端口
struct sockaddr_in send_addr[10];
struct sockaddr_in recv_addr;
static char sendIpAddress[10][128] ;  //各发送IP定义
struct sockaddr_in err_detection_addr ; 
int addr_len= -1 ;
int i = -1 ; 
int ret =-1 ;
static char ipaddr[19] ="IPADDRESS"; //ip缓冲区
static int sendCount= 0 ;  //发送的ip数目，最大为10
static int net_monitor_disable = 1 ;  //网络板检测禁止
static int net_monitor_Port= 0 ;  //网络板检测端口
static char net_monitor_ip[19] ="";  //网络板检测ip
static int m287_socket_config_init();  

int m287_socket_init(void)
{
    int flags;
    
    if(m287_socket_config_init()<0)
    {
        printf("网络配置文件读取错误\n");
        return -1 ;
    }

    //创建发送套接字
    socksed = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == socksed)
    {
        printf("setup socket fail\n");
        return -1;
    }

    //发送地址 
    for(i =0 ; i < sendCount ; i ++)
    {
        memset(&(send_addr[i]),0, sizeof(struct sockaddr_in));
        send_addr[i].sin_family = AF_INET;
        send_addr[i].sin_port = htons(sendPort);
        send_addr[i].sin_addr.s_addr = inet_addr(sendIpAddress[i]);
    }
    memset(&(err_detection_addr),0, sizeof(struct sockaddr_in));
    err_detection_addr.sin_family = AF_INET;
    err_detection_addr.sin_port = htons(net_monitor_Port);
    err_detection_addr.sin_addr.s_addr = inet_addr(net_monitor_ip);
   

     //创建接收套接字    
    sockrecv = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sockrecv)
    {
        printf("setup socket_recv fail\n");
        return -1;
    }
    //接收地址,本地址有两个用途。在此用于绑定本地接收数据的套接字信息。在接受数据函数中用于返回发送方的IP信息
    memset(&recv_addr,0, sizeof(struct sockaddr_in));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_addr.sin_port = htons(3001);  
    
    addr_len=sizeof(recv_addr);
    //将接受套接字绑定到本机任意ip的3001端口
    ret = bind(sockrecv,(struct sockaddr*)&recv_addr, sizeof(recv_addr)) ;
    if(ret < 0)
    {
        printf("bind fail\n");
        return -1;
    }
    //设置接收套接字非阻塞
    flags = fcntl(sockrecv, F_GETFL, 0);
    fcntl(sockrecv, F_SETFL, flags | O_NONBLOCK);

    return 0 ;
}

int m287_udpSend(const void *buf,size_t nbytes)
{
    if(sendCount >10)
    {
        printf("最多支持发送10个IP");
        return -1 ;
    }
    for(i =0 ; i < sendCount ; i ++)
    {
        ret = sendto(socksed,buf,nbytes,0,(struct sockaddr *)&(send_addr[i]),sizeof(struct sockaddr_in)); 
        if (ret == -1)
            return -1 ;
    }
    return 0 ;
}

int m287_errSend(const void *buf,size_t nbytes)
{
    if(net_monitor_disable)
        return 0;
     return  sendto(socksed,buf,nbytes,0,(struct sockaddr *)&(err_detection_addr),sizeof(struct sockaddr_in)); 
}


int m287_udpRecive(void * buf , size_t len)
{
    return recvfrom(sockrecv, buf, len, 0, (struct sockaddr*)&recv_addr,(socklen_t*)&addr_len);
}

void m287_socket_release(void)
{
    close(socksed);
    close(sockrecv);
}
    
static int m287_socket_config_init()
{
    sendCount = getConfValue("/opt/net.conf","SEND_COUNT",NULL,TYPE_INT);
    sendPort =  getConfValue("/opt/net.conf","PORT",NULL,TYPE_INT); 
    net_monitor_Port = getConfValue("/opt/net.conf","NET_MONITOR_PORT",NULL,TYPE_INT);
    net_monitor_disable = getConfValue("/opt/net.conf","NET_MONITOR_DISABLECARD",NULL,TYPE_INT);
    if(sendCount <0 || sendPort <0 || net_monitor_Port <0 || net_monitor_disable <0)
        return -1 ;
    for (i=0 ; i <sendCount ; i ++)  
    {
        sprintf(ipaddr,"IPADDRESS%d",(i+1));
        ret = getConfValue("/opt/net.conf",ipaddr,sendIpAddress[i],TYPE_STRING);
        if(ret < 0)        
            return -1 ;
    }    
    ret = getConfValue("/opt/net.conf","NET_MONITOR_IPADDRESS",net_monitor_ip,TYPE_STRING);
    if(ret < 0)        
        return -1 ;
   
    return 0 ;
}    
    
    
