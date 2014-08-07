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
#define UDP_PORT   (10010)


/*udp server*/
void udp_server(void)
{
  int err = -1, sock_reuse = 1;
  int udp_sock = -1;
  int recv_bytes = -1;
  rt_uint32_t addr_len;
  
  char udp_recv_buff[LEN];
  
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr; 
  
  MY_DEBUG("Now in  %s, %d\n\r",__func__,__LINE__);
  
  udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(udp_sock < 0) {
    MY_DEBUG("create udp socket faild !\n\r");
    goto OUT;
  }

  /*setsockopt*/
  err = setsockopt(udp_sock, SOL_SOCKET, 
                   SO_REUSEADDR, 
                   (const char*)&sock_reuse, sizeof(sock_reuse));
  
  if(err < 0) {
    MY_DEBUG("setsockopt faild!\n\r");
//    lwip_close(client_sock);
    goto OUT;
  }  
  
  /*bind server*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(UDP_PORT);
  server_addr.sin_addr.s_addr= INADDR_ANY;
  rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
  
  if(lwip_bind(udp_sock, 
              (struct sockaddr *)&server_addr, 
               sizeof(struct sockaddr)) < 0) {
                MY_DEBUG("bind client faild\n\r");
                goto OUT;
              }
  
  MY_DEBUG("Wate dta,udp:  %s, %d\n\r",__func__,__LINE__);
  /*recvfrom*/
  while(1) {
    rt_memset(udp_recv_buff, 0, LEN);
    rt_memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));
    recv_bytes = recvfrom(udp_sock, udp_recv_buff, LEN, 0, (struct sockaddr*)&client_addr, &addr_len);
    
    if(recv_bytes < 0) {
      MY_DEBUG("udp server recv error..\n\r");
      goto OUT;
    }
    MY_DEBUG("\n(%s , %d) said : ",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    MY_DEBUG("%s\n\r", udp_recv_buff);

    recv_bytes =  sendto(udp_sock, udp_recv_buff, sizeof(udp_recv_buff), 0,
              (struct sockaddr*)&client_addr, sizeof(struct sockaddr));    
   
    if(recv_bytes < 0) {
      MY_DEBUG("udp server send faild...\n\r");
      goto OUT;
    }
    rt_thread_delay(1000);
  }
  
OUT:
    lwip_close(udp_sock);
    udp_sock = -1;
    return ;
}

