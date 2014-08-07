#include <rtthread.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h>
#include <led.h>

#define YEELINK_HOSTNAME    "api.yeelink.net"

// Http��������
// �޸�U-ApiKey
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

    // ��һ�� DNS��ַ����
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
        // �ڶ��� �����׽��� 
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            rt_kprintf("Socket error\n"); 
            rt_free(recv_data);
            return;
        }
        
        // ������ ����yeelink 
        if (connect(sock, (struct sockaddr *)&yeelink_sockaddr, sizeof(struct sockaddr)) == -1)
        {
            rt_kprintf("Connect fail!\n");
            lwip_close(sock);
            rt_free(recv_data);
            return;
        }
        
        // ��4�� ����Http����
        send(sock,send_data,strlen(send_data), 0);
        
        // ��5�� ���Http��Ӧ
        bytes_received = recv(sock, recv_data, 1024 - 1, 0);
        recv_data[bytes_received] = '\0';
        
        // ��Ӧ����Ϊ {"timestamp":"2013-11-19T08:50:11","value":1}
        // ��ȡ��value��֮�������
        char* actuator_info = rt_strstr( recv_data , "\"value\"");
        int offset = rt_strlen("\"value\":");
        char actuator_status = *(actuator_info + offset);
        rt_kprintf("actuator status :%c\r\n",actuator_status);
        
        // ��ÿ���״̬��������LEDָʾ��
        (actuator_status == '1')?rt_hw_led_on(0):rt_hw_led_off(0);
        
        rt_memset(recv_data , 0 , sizeof(recv_data));
        
        // �ر��׽���
        closesocket(sock);
        // �ͷ�recv_data
        rt_free(recv_data);
//        // ��ʱ5S֮����������
//        rt_thread_delay( RT_TICK_PER_SECOND * 5 );
//    }

}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* ���httpclient������finsh shell�� */
FINSH_FUNCTION_EXPORT(httpclient, Get Actutor Information From Yeelink);
#endif
