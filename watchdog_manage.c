#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>

#include "config_manage.h"
#include "watchdog_manage.h"
/**WATCHDOG MANGER*/

static int wgfd = -1 ;
static const char *device = "/dev/watchdog";                             /* 看门狗设备驱动文件路径             */
static int wgt_timeout   = 2;                                               /* 默认配置的看门狗溢出时间     */
static int wgt_disable = 1 ;

static int set_timeout_watchdog(int timeout);
static int watchdog_config_init();

int m287_watchdog_init()
{
    if(watchdog_config_init() <0)
    {
        printf("看门狗初始化失败\n");
        return -1 ;
    }
    if (wgt_disable)
        return 0 ;
    wgfd = open(device, O_RDWR);                                    /* 读写方式打开设备文件         */
    if (wgfd < 0) {
        printf("device open failed!\r\n");                               /* 设备文件打开失败             */
        return -1;
    }
    else
    {
    //设备文件打开后看门狗随机被使能，溢出时间为内核中记录的上一次的设置值，首次设置默认为19秒
        printf("device open succeed, watchdog enable\n ");
        if(set_timeout_watchdog(wgt_timeout) < 0)
            return -1 ;
        else 
            printf("watchdog timeout is %d s \n" , wgt_timeout);
        return wgfd ;
    }
}

/* 禁能看门狗*/
int disable_WatchDog()
{
    int ret = -1 ;
    int opts = 0;
    // 禁能看门狗前需先写入'V'(WDT_OK_TO_CLOSE)，防止误关闭
    if (write(wgfd, "V", 1) != 1) {
        printf("write WDT_OK_TO_CLOSE failed!");
    }
    opts = WDIOS_DISABLECARD;
    ret = ioctl(wgfd, WDIOC_SETOPTIONS, &opts);          /* 关闭看门狗                   */
    printf("watchdog disable!\r\n");
    return ret ;
}

/* 设置看门狗加载值             */
static int set_timeout_watchdog(int timeout)
{
    return ioctl(wgfd, WDIOC_SETTIMEOUT, &timeout);                           /* 设置新的溢出时间             */
}

int m287_watchdog_keepalive()
{
    if (wgt_disable)
        return 0 ;
    else
        return ioctl(wgfd, WDIOC_KEEPALIVE, 0);                     /* 喂狗操作                     */
}
/* 关闭设备文件                 */
int m287_watchdog_closefd()
{
    int ret = close(wgfd);                                                    /* 关闭设备文件                 */
    if (ret != 0) {
        printf("device closed unexpectdly, watchdog keep on!\r\n");
    }
    return ret ;
}

static int watchdog_config_init()
{
    wgt_timeout = getConfValue("/opt/net.conf","WATCHDOG_TIMEOUT",NULL,TYPE_INT);
    wgt_disable = getConfValue("/opt/net.conf","WATCHDOG_DISABLECARD",NULL,TYPE_INT);
    if(wgt_timeout < 2 || wgt_disable < 0)
        return -1 ;
    else    
        return 0 ;

}

