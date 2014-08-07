#include <rtthread.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h>
#include <led.h>

#define YEELINK_HOSTNAME    "api.yeelink.net"

// Http请求内容
// 修改U-ApiKey
static const char send_data[] =
"GET /v1.0/device/1949/sensor/2511/datapoints HTTP/1.1\r\n"
"U-ApiKey: ffa3826972d6cc7ba5b17e104ec59fa3\r\n"
"Host: api.yeelink.net\r\n\r\n";

void httpclient(void)
{
    char *recv_data;
    int sock, bytes_received;   
    
    struct hostent *yeelink_host;
    struct in_addr yeelink_ipaddr;
    struct sockaddr_in yeelink_sockaddr;
    
    recv_data = rt_malloc(1024);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\r\n");
        return;
    }    

    // 第一步 DNS地址解析
    rt_kprintf("calling gethostbyname with: %s\r\n", YEELINK_HOSTNAME);  
    yeelink_host = gethostbyname(YEELINK_HOSTNAME);  
    yeelink_ipaddr.s_addr = *(unsigned long *) yeelink_host->h_addr_list[0];  
    rt_kprintf("Yeelink IP Address:%s\r\n" , inet_ntoa(yeelink_ipaddr));  
    
    yeelink_sockaddr.sin_family = AF_INET;
    yeelink_sockaddr.sin_port = htons(80);
    yeelink_sockaddr.sin_addr = yeelink_ipaddr;
    rt_memset(&(yeelink_sockaddr.sin_zero), 0, sizeof(yeelink_sockaddr.sin_zero));
    
//    while(1)
//    {
        // 第二步 创建套接字 
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            rt_kprintf("Socket error\n"); 
            rt_free(recv_data);
            return;
        }
        
        // 第三步 连接yeelink 
        if (connect(sock, (struct sockaddr *)&yeelink_sockaddr, sizeof(struct sockaddr)) == -1)
        {
            rt_kprintf("Connect fail!\n");
            lwip_close(sock);
            rt_free(recv_data);
            return;
        }
        
        // 第4步 发送Http请求
        send(sock,send_data,strlen(send_data), 0);
        
        // 第5步 获得Http响应
        bytes_received = recv(sock, recv_data, 1024 - 1, 0);
        recv_data[bytes_received] = '\0';
        
        // 响应内容为 {"timestamp":"2013-11-19T08:50:11","value":1}
        // 截取“value”之后的内容
        char* actuator_info = rt_strstr( recv_data , "\"value\"");
        int offset = rt_strlen("\"value\":");
        char actuator_status = *(actuator_info + offset);
        rt_kprintf("actuator status :%c\r\n",actuator_status);
        
        // 获得开关状态，并设置LED指示灯
        (actuator_status == '1')?rt_hw_led_on(0):rt_hw_led_off(0);
        
        rt_memset(recv_data , 0 , sizeof(recv_data));
        
        // 关闭套接字
        closesocket(sock);
        // 释放recv_data
        rt_free(recv_data);
//        // 延时5S之后重新连接
//        rt_thread_delay( RT_TICK_PER_SECOND * 5 );
//    }

}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* 输出httpclient函数到finsh shell中 */
FINSH_FUNCTION_EXPORT(httpclient, Get Actutor Information From Yeelink);
#endif
