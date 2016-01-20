#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include "config_manage.h"   


static char ipaddr[19] ="IPADDRESS"; 
static int i = 0;
static char sendIpAddress[5][128] ={"192.168.1.100","192.168.1.800","192.168.1.101","192.168.1.103","192.168.1.150"};  //各发送IP定义
static int sendPort= 0 ;
static int sendCount= 0 ;
static int wgt_t= 0 ;
static int wgt_d= 0 ;
static int net_monitor_Port= 0 ;
static char net_monitor_ip[19] ="";
static void m287_socket_config_init();
int main()  
{
    char ct[128]={0};
    int count = 0;
    //getConfValue("./net.conf","IPADDRESS1",ct,TYPE_STRING);   
    //printf("%s\n",ct); 
    
    m287_socket_config_init();
    
    for (i=0 ; i <5 ; i ++)  
    {
        printf("%s\n",sendIpAddress[i]);  
    } 
    printf("%d\n",sendPort); 
    printf("%d\n",net_monitor_Port); 
    printf("%d\n",sendCount); 
    printf("%d\n",wgt_t); 
    printf("%d\n",wgt_d);
    
    //printf("end\n"); 
} 

static void m287_socket_config_init()
{
    for (i=0 ; i <5 ; i ++)  
    {
        sprintf(ipaddr,"IPADDRESS%d",(i+1));
         
        getConfValue("./net.conf",ipaddr,sendIpAddress[i],TYPE_STRING);  
    }
    getConfValue("./net.conf","NET_MONITOR_IPADDRESS",net_monitor_ip,TYPE_STRING);
    sendPort =  getConfValue("./net.conf","PORT",NULL,TYPE_INT); 
    net_monitor_Port = getConfValue("./net.conf","NET_MONITOR_PORT",NULL,TYPE_INT);
    sendCount = getConfValue("./net.conf","SEND_COUNT",NULL,TYPE_INT);
    wgt_t = getConfValue("./net.conf","WATCHDOG_TIMEOUT",NULL,TYPE_INT);
    wgt_d = getConfValue("./net.conf","WATCHDOG_DISABLECARD",NULL,TYPE_INT);
} 
