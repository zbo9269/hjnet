#ifndef _socket_manage_h__
#define _socket_manage_h__

/*网络初始化*/
int m287_socket_init(void);

/*网络数据发送*/
int m287_udpSend(const void *buf,size_t nbytes);

/*网络检测发送*/
int m287_errSend(const void *buf,size_t nbytes);

/*网络数据接收*/
int m287_udpRecive(void * buf , size_t len);

/*网络*/
void m287_socket_release(void);

#endif
