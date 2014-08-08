#include <rtthread.h>
#include <lwip/sockets.h> /* 使用BSD Socket接口必须包含sockets.h这个头文件 */

int server_m2m_sock;
char m2m_recv[1024];
static const char send_data[] = "This is TCP Server from RT-Thread."; /* 发送用到的数据 */
void tcpserv(void* parameter)
{
 //  char *recv_data; /* 用于接收的指针，后面会做一次动态分配以请求可用内存 */
	rt_uint32_t sin_size;
	fd_set fdR;
	struct timeval timeout;
	int ret;
	int sock, bytes_received;
	struct sockaddr_in server_addr, client_addr;
	rt_bool_t stop = RT_FALSE; /* 停止标志 */
	int opt= 1;
  // recv_data = rt_malloc(1024); /* 分配接收用的数据缓冲 */
//   if (recv_data == RT_NULL)
//   {
//       rt_kprintf("No memory\n");
//       return;
//   }

   /* 一个socket在使用前，需要预先创建出来，指定SOCK_STREAM为TCP的socket */
   if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
       /* 创建失败的错误处理 */
       rt_kprintf("Socket error\n");

       /* 释放已分配的接收缓冲 */
 //      rt_free(recv_data);
       return;
   }

   /* 初始化服务端地址 */
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(5000); /* 服务端工作的端口 */
   server_addr.sin_addr.s_addr = INADDR_ANY;
   rt_memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero));

   /* 绑定socket到服务端地址 */
   if (lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
       /* 绑定失败 */
       rt_kprintf("Unable to bind\n");

       /* 释放已分配的接收缓冲 */
    //   rt_free(recv_data);
       return;
   }

   /* 在socket上进行监听 */
   if (lwip_listen(sock, 5) == -1)
   {
       rt_kprintf("Listen error\n");

       /* release recv buffer */
//       rt_free(recv_data);
       return;
   }

	while (1)
	{
		
		server_m2m_sock = lwip_accept(sock, (struct sockaddr *)&client_addr, &sin_size);
   
      		 rt_kprintf("I got a connection from (%s , %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		lwip_close(sock);
		lwip_setsockopt(server_m2m_sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
		
		while (1)
		{
		
			FD_ZERO(&fdR);
			FD_SET(server_m2m_sock,&fdR);

			timeout.tv_sec = 1;
			timeout.tv_usec= 0;
			ret = lwip_select(server_m2m_sock+1, &fdR, NULL, 0, &timeout);
			if (ret<= 0)
			{
				;
			}
			else if(FD_ISSET(server_m2m_sock, &fdR))
			{
				extern struct rt_mutex socket_data_buf_mutex;
				extern struct rt_mutex uart_data_buf_mutex;
				
				rt_mutex_take(&socket_data_buf_mutex,RT_WAITING_FOREVER);
				bytes_received =lwip_recv(server_m2m_sock,m2m_recv,1024, 0);
				 rt_mutex_release(&socket_data_buf_mutex);
			
				if (bytes_received<= 0)
				{
					lwip_close(server_m2m_sock);
					server_m2m_sock =-1;
				
					return ;
				}
				if(bytes_received > 0)
				{
					extern struct rt_device uart1_device;
				//	rt_mutex_take(&uart_data_buf_mutex,RT_WAITING_FOREVER);
					uart1_device.write(&uart1_device, 0, m2m_recv, bytes_received);
				//  	 rt_mutex_release(&uart_data_buf_mutex);
					//USART_printf(USART2,  recv_data);
					
					memset(m2m_recv, '\0', 1024);
				}
			
			}
		}
		lwip_close(server_m2m_sock);
		server_m2m_sock =-1;
		break;  
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* 输出tcpserv函数到finsh shell中 */
FINSH_FUNCTION_EXPORT(tcpserv, startup tcp server);
#endif

