/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include <lwip/sockets.h>
#endif
char rx_buf[1024];
struct rt_mutex socket_data_buf_mutex;

static void
tcp_server_thread(void *arg)
{
	void *parameter;
	extern void tcpclient(const char* url, int port);
	extern void tcpserv(void* parameter);
	extern void udpserv(void* paramemter);
	extern void udpclient(const char* url, int port, int count);
	while(1)
	{
		//tcpclient(parameter, 6000);		//tcp client
		tcpserv(parameter);		//tcp server
		//udpserv(paramemter);    //udp server
		//udpclient(paramemter, 5000,100);	//udp client
	}
}
static void
 uart_thread(void *arg)
 {
 	int len1;
	fd_set readset;
	int ret;
	struct timeval timeout;
 	extern struct rt_device uart1_device;
	while(1)
	{
	//	rt_mutex_take(&uart_data_buf_mutex,RT_WAITING_FOREVER);
		  len1 = rt_device_read(&uart1_device,0,rx_buf,1024);
	//	   rt_mutex_release(&uart_data_buf_mutex);
		  if(len1 > 0)
		  {
		  	//extern m2m_sock;
			extern server_m2m_sock;
			rx_buf[len1] = '\0';
			rt_mutex_take(&socket_data_buf_mutex,RT_WAITING_FOREVER);
			lwip_send(server_m2m_sock, rx_buf, len1, 0);
			 rt_mutex_release(&socket_data_buf_mutex);
			 memset(rx_buf, '\0', 1024);
		}
		  rt_thread_delay(10);
	}
 }

void rt_init_thread_entry(void* parameter)
{
/* LwIP Initialization */
#ifdef RT_USING_LWIP
	{
		extern void lwip_sys_init(void);

		/* register ethernetif device */
		eth_system_device_init();

#ifdef STM32F10X_CL
		rt_hw_stm32_eth_init();
#else
	/* STM32F103 */
	#if STM32_ETH_IF == 0
			rt_kprintf("rt_hw_enc28j60_init\n");
			rt_hw_enc28j60_init();
	#elif STM32_ETH_IF == 1
			rt_hw_dm9000_init();
	#endif
#endif
		rt_kprintf("rt_device_init_all\n");
		/* re-init device driver */
		rt_device_init_all();

		/* init lwip system */
		lwip_sys_init();
		rt_kprintf("TCP/IP initialized!\n");
		rt_mutex_init(&socket_data_buf_mutex,"musb",RT_IPC_FLAG_FIFO);

		 sys_thread_new("tcp server", tcp_server_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
		  sys_thread_new("uart", uart_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
	}
#endif
}

int rt_application_init()
{
	rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
	init_thread = rt_thread_create("init",
								rt_init_thread_entry, RT_NULL,
								2048, 8, 20);
#else
	init_thread = rt_thread_create("init",
								rt_init_thread_entry, RT_NULL,
								2048, 80, 20);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	return 0;
}

/*@}*/
