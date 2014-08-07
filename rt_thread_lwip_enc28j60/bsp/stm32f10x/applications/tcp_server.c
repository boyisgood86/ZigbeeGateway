#include <rtthread.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h>

#define _DEBUG   (1)
#if _DEBUG
  #define MY_DEBUG(fmt, args...) do{rt_kprintf(fmt, ##args);}while(0)
#else
  #define MY_DEBUG(fmt, args...)    do{}while(0)
#endif /*_DEBUG*/

#define LEN     (256)

//#define TEST_IP     ("192.168.1.125")
#define TCP_PORT   (10020)


unsigned char tcp_data[] = "Hello, I/m tcp server, it's just a demo!";


void tcp_server(void)
{
  int err = -1, sock_reuse = 1;
  int tcp_sock = -1 , do_sock = -1;
  int recv_bytes = -1;
  rt_uint32_t addr_len;
  
  char tcp_recv_buff[LEN];
  
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr; 
  
  MY_DEBUG("Now in  %s, %d\n\r",__func__,__LINE__);
  
  /*socket*/
  tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(tcp_sock < 0) {
    MY_DEBUG("%s, %d : create tcp server socket faild!\n\r",__func__,__LINE__);
    goto OUT;
  }

  /*setsockopt*/
  err = setsockopt(tcp_sock, SOL_SOCKET, 
                   SO_REUSEADDR, 
                   (const char*)&sock_reuse, sizeof(sock_reuse));
  
  if(err < 0) {
    MY_DEBUG("%s, %d: setsockopt faild!\n\r",__func__,__LINE__);
//    lwip_close(client_sock);
    goto OUT;
  }  
 
  /*bind*/
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(TCP_PORT); 
   server_addr.sin_addr.s_addr = INADDR_ANY;
   rt_memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero)); 
   if( lwip_bind(tcp_sock, (struct sockaddr *)&server_addr, 
                 sizeof(struct sockaddr)) < 0) {
                   MY_DEBUG("%s, %d: bind faild !\n\r",__func__,__LINE__);
                   goto OUT;
                 }
   
   /*listen*/
   if(lwip_listen(tcp_sock, 5) < 0) {
     MY_DEBUG("%s, %d: listen faild!\n\r",__func__,__LINE__);
     goto OUT;
   }
  /*accept*/
   while(1) {
      do_sock = lwip_accept(tcp_sock, (struct sockaddr *)&client_addr, &addr_len);
      if(do_sock < 0) {
        MY_DEBUG("%s, %d: accept error\n\r",__func__,__LINE__);
        goto OUT;
      }
      MY_DEBUG("%s, %d: i got a connect from :\n\r",__func__,__LINE__);
      MY_DEBUG("%s, %d\n\r",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
      lwip_close(tcp_sock);
      tcp_sock = -1;
      while(1) {
        rt_memset(tcp_recv_buff, 0, sizeof(tcp_recv_buff));
        recv_bytes = lwip_recv(do_sock, tcp_recv_buff, LEN, 0);
        if(recv_bytes < 0) {
          MY_DEBUG("%s, %d:> recv error\n\r",__func__,__LINE__);
          goto OUT;
        }else if(recv_bytes == 0) {
          MY_DEBUG("%s, %d:> client down..\n\r",__func__,__LINE__);
          lwip_close(do_sock);
          do_sock = -1;          
          break ;
        }else {
          MY_DEBUG("%s, %d:> recv data from client : %s\n\r",tcp_recv_buff);
          if(send(do_sock,tcp_recv_buff,sizeof(tcp_recv_buff), 0) < 0) {
            MY_DEBUG("%s, %d:> send data faild!\n\r",__func__,__LINE__);
            lwip_close(do_sock);
            do_sock = -1;
            goto OUT;
          }
        }
      }
   }
   
   
OUT:
  lwip_close(tcp_sock);
  tcp_sock = -1;
  return ;
}



