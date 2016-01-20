#ifndef _watchdog_manage_h__
#define _watchdog_manage_h__

/*看门狗初始化*/
int m287_watchdog_init();

/* 禁能看门狗*/
int disable_WatchDog();

/*喂狗*/
int m287_watchdog_keepalive();

/* 关闭设备文件*/
int m287_watchdog_closefd();


#endif