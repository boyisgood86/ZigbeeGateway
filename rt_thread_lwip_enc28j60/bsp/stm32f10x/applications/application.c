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
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

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

#include "led.h"
#include "enc28j60.h"

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[512];
static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;

    rt_hw_led_init();

    while (1)
    {
        /* led1 on */
#ifndef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        rt_hw_led_on(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        rt_kprintf("led off\r\n");
#endif
        rt_hw_led_off(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

void hello_thread(void *arg)
{
  while(1){
    rt_kprintf("Hello world!\n");
    rt_thread_delay(1000);
  }
}

static void tcp_client_thread(void * arg)
{
  extern void tcp_client(void);
  while(1) {
    tcp_client();
    rt_thread_delay(3000);
  }
}

static void udp_server_thread(void *arg)
{
  extern void udp_server(void);
  while(1) {
    udp_server();
    rt_thread_delay(1000);
  }
}

static void tcp_server_thread(void *arg)
{
  extern void tcp_server(void);
  while(1){
    tcp_server();
    rt_thread_delay(1000);
  }
}

void rt_init_thread_entry(void* parameter)
{
  rt_kprintf("%s, %d\n",__func__,__LINE__);
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
rt_kprintf("%s, %d\n",__func__,__LINE__);
#ifdef  RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */

rt_kprintf("%s, %d\n",__func__,__LINE__);    
    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    rt_kprintf("init enc28j60..\n");
    rt_hw_enc28j60_init();
    
    // 在控制台中启动
#if 0
    void httpclient(void);
    httpclient();
#endif
    sys_thread_new("hello", hello_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
    sys_thread_new("tcp_client", tcp_client_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
    sys_thread_new("udp_server", udp_server_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
    sys_thread_new("tcp_server", tcp_server_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
    
    
#endif

}

int rt_application_init(void)
{
    rt_thread_t init_thread;

    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }

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
