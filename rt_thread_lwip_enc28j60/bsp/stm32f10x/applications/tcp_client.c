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


#define TEST_IP     ("192.168.1.125")
#define TEST_PORT   (10000)

#define CLIENT_PORT     (10030)

unsigned char send_data[] = "Hello,I/m clinet, just test !";
char recv_buff[LEN];

int client_sock = -1;
int recv_bytes = -1;


/*tcp client demo*/
void tcp_client(void)
{
  int err = -1, sock_reuse = 1;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  
  MY_DEBUG("Now in  %s, %d\n\r",__func__,__LINE__);
  /*socket*/
  client_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(client_sock < 0){
    MY_DEBUG("create socket faild!\n\r");
    goto OUT;
  }
  
  /*setsockopt*/
  err = setsockopt(client_sock, SOL_SOCKET, 
                   SO_REUSEADDR, 
                   (const char*)&sock_reuse, sizeof(sock_reuse));
  
  if(err < 0) {
    MY_DEBUG("setsockopt faild!\n\r");
//    lwip_close(client_sock);
    goto OUT;
  }
  
//  err = setsockopt(client_sock, SOL_SOCKET, 
//                   SO_REUSEPORT, 
//                   (const char*)&sock_reuse, sizeof(sock_reuse));
//  
//  if(err < 0) {
//    MY_DEBUG("setsockopt faild!\n\r");
////    lwip_close(client_sock);
//    goto OUT;
//  }  
 
  /* bind client*/
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(CLIENT_PORT);
  client_addr.sin_addr.s_addr = INADDR_ANY;
  rt_memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));  
  if(lwip_bind(client_sock, 
              (struct sockaddr *)&client_addr, 
               sizeof(struct sockaddr)) < 0) {
                MY_DEBUG("bind client faild\n\r");
                goto OUT;
              }
  
/*connect server*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(TEST_PORT);
  server_addr.sin_addr.s_addr=inet_addr(TEST_IP);
  rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
  
  if(connect(client_sock, 
            (struct sockaddr *)&server_addr, 
             sizeof(struct sockaddr)) < 0) {
                MY_DEBUG("can't connect..\n\r");
                goto OUT;
            }
  MY_DEBUG("connect success: %s, %d\n", __func__,__LINE__);
 /*send*/ 
  if( send(client_sock, send_data, sizeof(send_data), 0) < 0) {
      MY_DEBUG("send data to server faild !\n\r");
      goto OUT;
  }
  
 /*recv*/ 
  rt_memset(recv_buff, 0, LEN);
  recv_bytes = recv(client_sock, recv_buff, LEN, 0);
  if(recv_bytes < 0) {
    MY_DEBUG("recv error...\n\r");
    goto OUT;
  }else if(recv_bytes == 0) {
    MY_DEBUG("server down...\n\r");
  }else {
    MY_DEBUG("recv data from server is %s\n",recv_buff );
  }
    
    lwip_close(client_sock);
    client_sock = -1;
    return ;    
  
  
OUT:
    lwip_close(client_sock);
    client_sock = -1;
    return ;  
}

