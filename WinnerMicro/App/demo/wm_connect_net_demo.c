#include <string.h>
#include "wm_include.h"
#include "wm_http_fwup.h"

#if DEMO_CONNECT_NET
extern const char DEMO_ONESHOT[];
extern const char DEMO_CONNET[];
#define    DEMO_ONESHOT_TASK_SIZE      128
OS_EVENT *demo_oneshot_q = NULL;
void *demo_oneshot_queue[DEMO_QUEUE_SIZE];
static OS_STK DemoOneShotTaskStk[DEMO_ONESHOT_TASK_SIZE]; 
extern ST_Demo_Sys gDemoSys;
u8 ifoneshot = 0;

static void demo_oneshot_task(void *sdata);
static int CreateOneshotDemoTask(void);

static void con_net_status_changed_event(u8 status )
{
	struct tls_ethif * ethif;
	
	switch(status)
	{
		case NETIF_WIFI_JOIN_FAILED:
			printf("\nconnect net: join net failed\n");
			break;
		case NETIF_WIFI_JOIN_SUCCESS:
			printf("\nconnect net: join net success\n");
			break;
		case NETIF_IP_NET_UP:
			ethif = tls_netif_get_ethif();
			printf("\nip=%d.%d.%d.%d\n",ip4_addr1(&ethif->ip_addr.addr),ip4_addr2(&ethif->ip_addr.addr),
					ip4_addr3(&ethif->ip_addr.addr),ip4_addr4(&ethif->ip_addr.addr));

			if(ifoneshot)	//�����һ�����ã���һ��������������Ϣ����Ϣ�����а�MAC��ַ�����ֻ�app
			{
				ifoneshot = 0;
				OSQPost(demo_oneshot_q,(void *)DEMO_MSG_SOCKET_CREATE);
			}
			break;
		default:
			break;
	}
}


//����demo�����ݲ����ж���һ����������������������
//һ����������ʾ��:t-oneshot
//������������ʾ��:t-connet("ssid","pwd");
int DemoConnectNet(char *buf)
{	
	if(strstr(buf,DEMO_ONESHOT) != NULL)	//һ������
	{
		ifoneshot = 1;
		CreateOneshotDemoTask();
	}
	else if(strstr(buf, DEMO_CONNET) != NULL)
	{
		char *p1 = NULL,*p2 = NULL;
		char ssid[64];
		char pwd[64];
		struct tls_param_ip ip_param;
		
		if(strchr(buf, ';') != NULL || strchr(buf,')') != NULL)		//�յ������������
		{
			memset(ssid,0,sizeof(ssid));
			memset(pwd,0,sizeof(pwd));
			p1 = strchr(buf,'"');
			if(NULL == p1)
				return WM_FAILED;
			p2 = p1 +1;	//ssid����ʼλ��
			p1 = strchr(p2,'"');	//ssid�Ľ���λ��
			if(NULL == p1)
				return WM_FAILED;
			memcpy(ssid, p2, p1 - p2);
			printf("\nssid=%s\n",ssid);
			p2 = p1 + 1;
			p1 = strchr(p2,'"');	
			if(NULL == p1)
				return WM_FAILED;
			p2 = p1 + 1;		//pwd ����ʼλ��
			p1 = strchr(p2, '"');	//pwd�Ľ���λ��
			if(NULL == p1)
				return WM_FAILED;
			memcpy(pwd, p2, p1 - p2);
			printf("\npassword=%s\n",pwd);
			tls_wifi_set_oneshot_flag(0);
			ifoneshot = 0;

			tls_param_get(TLS_PARAM_ID_IP, &ip_param, FALSE);
			ip_param.dhcp_enable = true;
			tls_param_set(TLS_PARAM_ID_IP, &ip_param, FALSE);
			
			tls_wifi_connect((u8 *)ssid, strlen(ssid), (u8 *)pwd, strlen(pwd));
			tls_netif_add_status_event(con_net_status_changed_event);
			printf("\nplease wait connect net......\n");
		}
		else
			return DEMO_CONSOLE_SHORT_CMD;
	}

	return WM_SUCCESS;
}


static int CreateOneshotDemoTask(void)
{
	demo_oneshot_q = OSQCreate((void **)&demo_oneshot_queue, DEMO_QUEUE_SIZE);
	tls_os_task_create(NULL, NULL,
			demo_oneshot_task,
                    (void *)&gDemoSys,
                    (void *)DemoOneShotTaskStk,          /* ����ջ����ʼ��ַ */
                    DEMO_ONESHOT_TASK_SIZE * sizeof(u32), /* ����ջ�Ĵ�С     */
                    DEMO_ONESHOT_TASK_PRIO,
                    0);

	return WM_SUCCESS;
}


static void demo_oneshot_task(void *sdata)
{
	//ST_Demo_Sys *sys = (ST_Demo_Sys *)sdata;
	void *msg;
	INT8U error;
	
	tls_wifi_set_oneshot_flag(1);		/*һ������ʹ��*/
	tls_netif_add_status_event(con_net_status_changed_event);
	printf("\nwait one shot......\n");
	for(;;) 
	{
    		msg = OSQPend(demo_oneshot_q, 0, &error);
		//printf("\n msg =%d\n",msg);
		switch((u32)msg)
		{
			case DEMO_MSG_WJOIN_SUCCESS:
				break;
				
			case DEMO_MSG_SOCKET_CREATE:
#if TLS_CONFIG_SOCKET_STD
				DemoStdSockOneshotSendMac();		//��ѡһ��		
#endif
#if TLS_CONFIG_SOCKET_RAW
				//DemoRawSockOneshotSendMac();
#endif //TLS_CONFIG_SOCKET_RAW
				break;
				
			case DEMO_MSG_WJOIN_FAILD:
				break;
			default:
				break;
		}
	}

}

#endif //DEMO_CONNECT_NET

#if DEMO_CONSOLE
#if TLS_CONFIG_SOCKET_STD
//one shot�����ɹ�֮�󣬰�MAC��ַ�ù㲥�����ͳ�ȥ��֪ͨ�ֻ�app
void DemoStdSockOneshotSendMac(void)
{
	struct sockaddr_in pin;
	int idx;
	int socket_num;
	u8 mac_addr[8];

	memset(&pin, 0, sizeof(struct sockaddr));
	pin.sin_family=AF_INET;                 //AF_INET��ʾʹ��IPv4
	pin.sin_addr.s_addr=htonl(0xffffffffUL);  //IPADDR_BROADCAST
	pin.sin_port=htons(65534);
	socket_num = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	//printf("\nstd sk one shot sock num=%d\n",socket_num);

	memset(mac_addr,0,sizeof(mac_addr));
	tls_get_mac_addr(mac_addr);
	OSTimeDly(200);				
	for(idx = 0;idx < 3;idx++)
	{
		sendto(socket_num, mac_addr, 6, 0, (struct sockaddr *)&pin, sizeof(struct sockaddr));
		OSTimeDly(50);
		//printf("========> socket num=%d\n",socket_num);
	}
	closesocket(socket_num);
	//printf("\none shot success!\n");
}
#endif //TLS_CONFIG_SOCKET_STD
#if TLS_CONFIG_SOCKET_RAW
void DemoRawSockOneshotSendMac(void)
{
	int idx;
	int socket_num = 0;
	u8 mac_addr[8];
	struct tls_socket_desc socket_desc;

	memset(&socket_desc, 0, sizeof(struct tls_socket_desc));
	socket_desc.cs_mode = SOCKET_CS_MODE_CLIENT;
	socket_desc.protocol = SOCKET_PROTO_UDP;
	for(idx = 0; idx < 4; idx++){
		socket_desc.ip_addr[idx] = 255;
	}
	socket_desc.port = 65534;
	socket_num = tls_socket_create(&socket_desc);
	//printf("\nraw sk one shot sock num=%d\n",socket_num);
	
	memset(mac_addr,0,sizeof(mac_addr));
	tls_get_mac_addr(mac_addr);
	OSTimeDly(200);				
	for(idx = 0;idx < 3;idx ++)
	{
		tls_socket_send(socket_num,mac_addr, 6);
		OSTimeDly(50);
	}
	tls_socket_close(socket_num);
	socket_num = 0;	
	//printf("\none shot success!\n");
}
#endif //TLS_CONFIG_SOCKET_RAW

#endif

