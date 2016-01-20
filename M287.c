#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "spidev.h"
#include "watchdog_manage.h"
#include "socket_manage.h"


#define ERR_LOG_BUFF_SIZE 1024

static const char *gpio2_4 = "/sys/class/gpio/gpio68/value";
static const char *gpio2_5 = "/sys/class/gpio/gpio69/value";
static const char *gpio2_6 = "/sys/class/gpio/gpio70/value";
static const char *gpio2_7 = "/sys/class/gpio/gpio71/value";

static const char *gpio2_12 = "/sys/class/gpio/gpio76/value";
static const char *gpio2_13 = "/sys/class/gpio/gpio77/value";
static const char *gpio2_14 = "/sys/class/gpio/gpio78/value";
static const char *gpio2_15 = "/sys/class/gpio/gpio79/value";

static const char *gpio3_7 = "/sys/class/gpio/gpio103/value";

static const char *gpio3_15 = "/sys/class/gpio/gpio111/value";
static const char *gpio3_27 = "/sys/class/gpio/gpio123/value";
static const char *spi_device = "/dev/spidev2.0";                        /* SPI设备驱动文件路径             */

static int err_log_buf_len = 0;   //错误数据长度
static char err_log_buf[ERR_LOG_BUFF_SIZE] = {0};  //错误数据缓冲区

static void pabort(const char *s)
{
    m287_errSend(s,sizeof(s));     
    perror(s);
    abort();
}

static void clear_err_buf()
{
    memset(err_log_buf,0,ERR_LOG_BUFF_SIZE);
    err_log_buf_len = 0 ;
}

int main()
{  
    int ret = -1;   //返回值变量
    int fd= -1;   //SPI文件描述符
    int fd204,fd205,fd206,fd207,fd212,fd213,fd214,fd215,fd307,fd315,fd327;   //GPIO管脚文件描述符
    int i;
    const unsigned char ip_a = 192 ;
    const unsigned char ip_b = 168 ;
    const int lens=3000;    //SPI通讯最大数据长度
    int recv_len=0;         //网络接收数据实际长度

    unsigned int cnt=0;   //SPI通信实际数据长度
    unsigned char value=0;  //FPGA有数标识
    /**SPI 参数设置**/
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 20000000;
    uint16_t delay = 0;
    
    struct spi_ioc_transfer transfer;
    struct spi_ioc_transfer receiver;
    
    uint8_t tx[lens];    //SPI通信
    uint8_t rx[lens];    //SPI通信
    uint8_t sendbuf[lens];    //网络数据发送缓冲区
    uint8_t recivebuf[lens];    //网络数据接收缓冲区

    memset(tx,0,sizeof(tx));
    memset(rx,0,sizeof(rx));
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recivebuf,0,sizeof(recivebuf));
    
    /**Init Socket**/
    if (m287_socket_init() <0)
    {
        printf("网络初始化失败\n");
        return 0 ;
    }
    
    /**Init watchdog**/
    if(m287_watchdog_init()<0)
    {
        printf("看门狗初始化失败\n");
        return 0 ;
    }
 
    /************Init gpio3.7|gpio3.15| gpio3.27***************/
    fd315 = open(gpio3_15, O_RDWR);
    if (fd315 < 0)
        pabort("can't open gpio3.15_device");
    fd327 = open(gpio3_27, O_RDWR);
    if (fd327 < 0)
        pabort("can't open gpio3.27_device");
    fd307 = open(gpio3_7, O_RDWR);
    if (fd307 < 0)
        pabort("can't open gpio3.7_device");        
    
    /************Init gpio2.4~~gpio2.7***************/
    fd204 = open(gpio2_4, O_RDWR);
    if (fd204 < 0)
        pabort("can't open gpio2.4_device");
    fd205 = open(gpio2_5, O_RDWR);
    if (fd205 < 0)
        pabort("can't open gpio2.5_device");
    fd206 = open(gpio2_6, O_RDWR);
    if (fd206 < 0)
        pabort("can't open gpio2.6_device");
    fd207 = open(gpio2_7, O_RDWR);
    if (fd207 < 0)
        pabort("can't open gpio2.7_device");

    /************Init gpio2.12~~gpio2.15***************/
    fd212 = open(gpio2_12, O_RDWR);
    if (fd212 < 0)
        pabort("can't open gpio2.12_device");
    fd213 = open(gpio2_13, O_RDWR);
    if (fd213 < 0)
        pabort("can't open gpio2.13_device");
    fd214 = open(gpio2_14, O_RDWR);
    if (fd214 < 0)
        pabort("can't open gpio2.14_device");
    fd215 = open(gpio2_15, O_RDWR);
    if (fd215 < 0)
        pabort("can't open gpio2.15_device");
    
    /************Init SPI*************/   
    fd = open(spi_device, O_RDWR);
    if (fd < 0)
        pabort("can't open spi_device");

    /*
     * spi mode
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set wr spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");

    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

    transfer.tx_buf = (unsigned long)tx;
    transfer.rx_buf = 0;
    transfer.len = lens;
    transfer.delay_usecs = delay;
    transfer.speed_hz = speed;
    transfer.bits_per_word = bits;

    receiver.tx_buf = 0;
    receiver.rx_buf = (unsigned long)rx;
    receiver.len = lens;
    receiver.delay_usecs = delay;
    receiver.speed_hz = speed;
    receiver.bits_per_word = bits;

    m287_errSend("net starting....",15);    
    while(1)
    {
        lseek(fd315, 0, SEEK_SET);
        read(fd315, &value, 1);        
        if(value=='1')
        {
            write(fd206, "0", 1);
            write(fd205, "0", 1);
            write(fd204, "1", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
        
            write(fd206, "0", 1);
            write(fd205, "1", 1);
            write(fd204, "0", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
            
            receiver.len = 8;
            ret = ioctl(fd, SPI_IOC_MESSAGE(1), &receiver);        
            if (ret < 0) 
                pabort("can't receive spi message");
            cnt = (rx[0]<<8)+rx[1];  //通过SPI从FPGA中得到数据长度
            if (cnt > 2900 || cnt <= 30 || rx[2] != ip_a || rx[3] != ip_b)   //数据长度大于预定长度，直接进行下一次循环
            {
                if (cnt >2900)
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data overflow , len from FPGA is %d",cnt);
                }
                if (cnt ==0)
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data LEN is 0");
                }
                if (rx[2] != ip_a || rx[3] != ip_b)
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data ip err , ip from FPGA is %d-%d",rx[2],rx[3]);
                }
                else
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data len too short , len from FPGA is %d",cnt);
                }
                m287_errSend(err_log_buf,err_log_buf_len);
                clear_err_buf();
                continue ;
                
            }
            else    
                printf("recive from FPGA and send to Hmi %d lens\n" , cnt);  
            
            write(fd206, "0", 1);
            write(fd205, "1", 1);
            write(fd204, "1", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
             
            receiver.len = cnt+1;
            ret = ioctl(fd, SPI_IOC_MESSAGE(1), &receiver);        
            if (ret < 0) 
                pabort("can't receive spi message");
               
            write(fd206, "0", 1);
            write(fd205, "0", 1);
            write(fd204, "0", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
            
            for(i=0;i<cnt;i++)
            {
                sendbuf[i] = rx[i+1]; 
                //printf("data[%02d] is 0x%02x\n",i,sendbuf[i]);
            }            
            //udp发送数据
             m287_udpSend(sendbuf,cnt) ;                                          
        }
        
        lseek(fd327, 0, SEEK_SET);
        read(fd327, &value, 1);        
        if(value=='1')
        {
            write(fd206, "1", 1);
            write(fd205, "0", 1);
            write(fd204, "1", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
            
            write(fd206, "1", 1);
            write(fd205, "1", 1);
            write(fd204, "0", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
            
            receiver.len = 8;
            ret = ioctl(fd, SPI_IOC_MESSAGE(1), &receiver);        
            if (ret < 0) 
                pabort("can't receive spi message");
            cnt = (rx[0]<<8)+rx[1];
            if (cnt > 2900 || cnt <= 30 || rx[2] != ip_a || rx[3] != ip_b)   //FPGA 数据有错误，直接进行下一次循环
            {
                if (cnt >2900)
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data overflow , len from FPGA is %d",cnt);
                }
                if (cnt ==0)
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data LEN is 0");
                }
                if (rx[2] != ip_a || rx[3] != ip_b)
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data ip err , ip from FPGA is %d-%d",rx[2],rx[3]);
                }
                else
                {
                    err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                        ERR_LOG_BUFF_SIZE - err_log_buf_len,
                        "FPGA data len too short , len from FPGA is %d",cnt);
                }
                m287_errSend(err_log_buf,err_log_buf_len);
                clear_err_buf();
                continue ;                
            }
            else    
                printf("recive from FPGA and send to Hmi %d lens\n" , cnt);        
        
            write(fd206, "1", 1);
            write(fd205, "1", 1);
            write(fd204, "1", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);
            
            receiver.len = cnt+1;
            ret = ioctl(fd, SPI_IOC_MESSAGE(1), &receiver);        
            if (ret < 0) 
                pabort("can't receive spi message");
            
            write(fd206, "0", 1);
            write(fd205, "0", 1);
            write(fd204, "0", 1);
            write(fd207, "1", 1);
            write(fd207, "0", 1);

            for(i=0;i<cnt;i++)  //由于FPGA程序问题，数据向右偏移了一个字节，这里偏移回去，得到真是数据
            {
                sendbuf[i] = rx[i+1]; 
                //printf("data[%02d] is 0x%02x\n",i,sendbuf[i]);
            }            
           
            //udp发送数据
            m287_udpSend(sendbuf,cnt) ;           
        }
        recv_len = m287_udpRecive(tx , lens); 
        if(recv_len>20 && recv_len < 50)
        {
            //帧头55AA ，tx[11]为接收战场编号
            if(tx[0]==0x55 && tx[1]==0xaa && tx[11]>0x0 &&tx[11]<0x0b)
            {
                write(fd215, "1", 1);
                write(fd214, "1", 1);
                write(fd213, "0", 1);
                write(fd212, "0", 1);
                write(fd307, "1", 1);
                write(fd307, "0", 1);

                if(tx[11]==0x01)
                {
                    write(fd215, "0", 1);
                    write(fd214, "0", 1);
                    write(fd213, "0", 1);
                    write(fd212, "1", 1);
                }
                else if(tx[11]==0x02)
                {
                    write(fd215, "0", 1);
                    write(fd214, "0", 1);
                    write(fd213, "1", 1);
                    write(fd212, "0", 1);
                }
                else if(tx[11]==0x03)
                {
                    write(fd215, "0", 1);
                    write(fd214, "0", 1);
                    write(fd213, "1", 1);
                    write(fd212, "1", 1);
                }
                else if(tx[11]==0x04)
                {
                    write(fd215, "0", 1);
                    write(fd214, "1", 1);
                    write(fd213, "0", 1);
                    write(fd212, "0", 1);
                }                
                else if(tx[11]==0x05)
                {
                    write(fd215, "0", 1);
                    write(fd214, "1", 1);
                    write(fd213, "0", 1);
                    write(fd212, "1", 1);
                }
                else if(tx[11]==0x06)
                {
                    write(fd215, "0", 1);
                    write(fd214, "1", 1);
                    write(fd213, "1", 1);
                    write(fd212, "0", 1);
                }
                else if(tx[11]==0x07)
                {
                    write(fd215, "0", 1);
                    write(fd214, "1", 1);
                    write(fd213, "1", 1);
                    write(fd212, "1", 1);
                }
                else if(tx[11]==0x08)
                {
                    write(fd215, "1", 1);
                    write(fd214, "0", 1);
                    write(fd213, "0", 1);
                    write(fd212, "0", 1);
                }                
                else if(tx[11]==0x09)
                {
                    write(fd215, "1", 1);
                    write(fd214, "0", 1);
                    write(fd213, "0", 1);
                    write(fd212, "1", 1);
                }   

                else if(tx[11]==0x0a)
                {
                    write(fd215, "1", 1);
                    write(fd214, "0", 1);
                    write(fd213, "1", 1);
                    write(fd212, "0", 1);
                }
                write(fd307, "1", 1);
                write(fd307, "0", 1);
                
                transfer.len = recv_len;
                ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
                if (ret < 0) 
                    pabort("can't send spi message");

                write(fd215, "1", 1);
                write(fd214, "1", 1);
                write(fd213, "1", 1);
                write(fd212, "0", 1);
                write(fd307, "1", 1);
                write(fd307, "0", 1);

                write(fd215, "0", 1);
                write(fd214, "0", 1);
                write(fd213, "0", 1);
                write(fd212, "0", 1);
                write(fd307, "1", 1);
                write(fd307, "0", 1);  
            }
            
            //控显机数据帧头错误
            else
            {
                err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                    ERR_LOG_BUFF_SIZE - err_log_buf_len,
                    "HMI data err , data from HMI is %s", tx);

                m287_errSend(err_log_buf, err_log_buf_len);
                clear_err_buf();
            }
           
        }
       
        //控显机数据长度错误
        else
        {
            err_log_buf_len += snprintf(err_log_buf + err_log_buf_len,
                ERR_LOG_BUFF_SIZE - err_log_buf_len,
                "HMI data len err , len from HMI is %d,data is %s",cnt,tx);
        }

        
        m287_watchdog_keepalive();//喂狗
        cnt = 0 ;
        clear_err_buf();
        usleep(1000);    //延时一毫秒
    }
    close(fd);
    close(fd204); 
    close(fd205);
    close(fd206);  
    close(fd207);
    close(fd212);
    close(fd213);
    close(fd214);
    close(fd215); 
    close(fd307);    
    close(fd315);
    close(fd327); 
    m287_socket_release();
    m287_watchdog_closefd();
}







