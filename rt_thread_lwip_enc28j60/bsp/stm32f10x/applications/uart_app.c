/*
*
* 只是一个使用uart1 设备的demo 文件
*
*/

#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include <lwip/sockets.h>
#include <lwip/dns.h>
#endif


#define _DEBUG   (1)
#if _DEBUG
  #define MY_DEBUG(fmt, args...) do{rt_kprintf(fmt, ##args);}while(0)
#else
  #define MY_DEBUG(fmt, args...)    do{}while(0)
#endif /*_DEBUG*/

#define     USART   ("uart1") 

#define     LEN     (1024)

extern int client_sock;


unsigned char tx_buff[] = "Hello, I/m usart 1 ,just demo !\n\r";
char rx_buff[LEN];


void usart1_tcpip(void)
{
   rt_device_t usart_device;
   rt_err_t result = RT_EOK;  
   int read_size = -1;

#ifdef RT_USING_UART1   
   /*find uart1*/
   usart_device = rt_device_find(USART);
   if (usart_device == RT_NULL) {
      MY_DEBUG("Can't find device %s \n\r", USART);
      goto OUT;
   } 
//      rt_device_open(usart_device, RT_DEVICE_OFLAG_RDWR);  
    while(1) {
      rt_memset(rx_buff, 0, LEN);
      read_size = rt_device_read(usart_device, 0, rx_buff, LEN);
      if(read_size > 0) {
        rt_device_write(usart_device, 0, rx_buff, read_size);
        MY_DEBUG("Now tcp client fd is %d\n\r", client_sock);
        if(client_sock >= 0) {
          if(lwip_send(client_sock, rx_buff, read_size,0) < 0) {
            MY_DEBUG("%s , %d : send faild!\n\r",__func__,__LINE__);
          }
          lwip_send(client_sock, "\n\r", 4, 0);
        }
        
        MY_DEBUG("\n\r");
      }else {
        rt_device_write(usart_device, 0, tx_buff, sizeof(tx_buff));
      }
      rt_thread_delay(1000);
    }
#endif
   return ;
OUT:  
  return ;
}



//struct rx_msg
//{
//    rt_device_t dev;
//    rt_size_t   size;
//};
// 
//static struct rt_messagequeue  rx_mq;
//static char uart_rx_buffer[64];
//static char msg_pool[2048];
// 
//// 串口侦听回调函数
//rt_err_t uart_input(rt_device_t dev, rt_size_t size)
//{
//    struct rx_msg msg;
//    msg.dev = dev;
//    msg.size = size;
//   
//        // 将接收内容放入消息队列
//    rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
//   
//    return RT_EOK;
//}
// 
//// 任务入口函数
//void usr_echo_thread_entry(void* parameter)
//{
//    struct rx_msg msg;
//   
//    rt_device_t device;
//    rt_err_t result = RT_EOK;
//   
//    
//    // 从RT系统中获取串口1设备
//    device = rt_device_find("uart1");
//    if (device != RT_NULL)
//    {
//                           // 指定接收串口内容的回调函数
//        rt_device_set_rx_indicate(device, uart_input);
//                           // 以读写方式打开设备
//        rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
//    }
//    MY_DEBUG("Now : %s, %d \n\r",__func__,__LINE__);
//   
//    while(1)
//    {
//                           // 从消息队列中获取被回调函数放入消息队列中的内容
//        result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 50);
//        if (result == -RT_ETIMEOUT)
//        {
//            // timeout, do nothing
//        }
//       
//        if (result == RT_EOK)
//        {
//            rt_uint32_t rx_length;
//           
//            rx_length = (sizeof(uart_rx_buffer) - 1) > msg.size ?
//                msg.size : sizeof(uart_rx_buffer) - 1;
//           
//            rx_length = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length);
//            uart_rx_buffer[rx_length] = '\0';
//            // 将内容写回到串口1
//            rt_device_write(device, 0, &uart_rx_buffer[0], rx_length);
//        }
//    }
//}
//// 串口例程初始化函数
//void usr_echo_init(void)
//{
//    rt_thread_t thread ;
//   
//    rt_err_t result; 
//      // 创建消息队列，分配队列存储空间
//    result = rt_mq_init(&rx_mq, "mqt", &msg_pool[0], 128 - sizeof(void*), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
//   
//    if (result != RT_EOK) 
//    { 
//        rt_kprintf("init message queue failed.\n"); 
//        return; 
//    } 
//    // 创建任务线程
//    thread = rt_thread_create("devt",
//        usr_echo_thread_entry, RT_NULL,
//        1024, 25, 7);
//    // 启动任务线程
//    if (thread != RT_NULL) {
//        MY_DEBUG("Now start usart 1 thread!  :%s, %d\n",__func__,__LINE__);
//        rt_thread_startup(thread);
//    }
//}
 







