#ifndef __WM_DEMO_H__
#define __WM_DEMO_H__

#define DEMO_ON		1
#define DEMO_OFF		0

//demo ����̨����ʾdemo��ʱ�����򿪸���
#define DEMO_CONSOLE				DEMO_ON

//socket demo����Ϊraw�ӿںͱ�׼�ӿ�,�����5�������ͬʱ�򿪣�
//��ʾ��ʱ����ʾ��һ��socket demo��ģ����Ҫ��λ������ʾ��һ��socket demo
#define DEMO_RAW_SOCKET_CLIENT	(DEMO_OFF && TLS_CONFIG_SOCKET_RAW) 	//raw �ӿ�socket client
#define DEMO_RAW_SOCKET_SERVER	(DEMO_OFF && TLS_CONFIG_SOCKET_RAW) 	//raw �ӿ�socket server
#define DEMO_STD_SOCKET_CLIENT	(DEMO_OFF && TLS_CONFIG_SOCKET_STD)	//��׼�ӿ�socket client
#define DEMO_STD_SOCKET_SERVER	(DEMO_OFF && TLS_CONFIG_SOCKET_STD)	//��׼�ӿ�socket server
#define DEMO_STD_SOCKET_SER_SEL    (DEMO_OFF && TLS_CONFIG_SOCKET_STD)    //��׼�ӿ�socket server, select ��ʽ

//����demo��������һ������������Ҳ���������ýӿ�����
#define DEMO_CONNECT_NET			DEMO_ON

//�ಥ�㲥demo
#define DEMO_UDP_MULTI_CAST        	(DEMO_OFF && TLS_CONFIG_SOCKET_STD) 

//socket ����demo
#define DEMO_SOCKET_FWUP      		(DEMO_OFF && TLS_CONFIG_SOCKET_RAW)

//AP dmeo
#define DEMO_SOFT_AP   		  		(DEMO_OFF && TLS_CONFIG_AP)

//WPS demo	
#define DEMO_WPS			  		(DEMO_OFF && TLS_CONFIG_WPS && TLS_IEEE8021X_EAPOL)

//http demo
#define DEMO_HTTP					(DEMO_OFF && TLS_CONFIG_HTTP_CLIENT)

//gpio demo
#define DEMO_GPIO					DEMO_OFF

//flash demo
#define DEMO_FLASH					DEMO_OFF

//master spi demo
#define DEMO_MASTER_SPI			DEMO_OFF

//slave spi demo
#define DEMO_SLAVE_SPI				(DEMO_OFF && TLS_CONFIG_HS_SPI)

//�ӽ���demo
#define DEMO_ENCRYPT				DEMO_OFF
//kii cloud demo
#define DEMO_KII				       DEMO_ON

////////////////////////////////////////////////////////////////

#define RemotePort	1000	//demo��Ϊclientʱ��Զ�̶˿�
#define LocalPort		1020	//demo��Ϊserverʱ�����ض˿�


// user prio 32 - 60
#define DEMO_TASK_PRIO			32
#define  DEMO_RAW_SOCKET_C_TASK_PRIO	(DEMO_TASK_PRIO + 1)
#define  DEMO_RAW_SOCKET_S_TASK_PRIO	(DEMO_RAW_SOCKET_C_TASK_PRIO + 1)
#define  DEMO_SOCKET_C_TASK_PRIO	(DEMO_RAW_SOCKET_S_TASK_PRIO + 1)
#define  DEMO_SOCKET_S_TASK_PRIO	(DEMO_SOCKET_C_TASK_PRIO + 1)
#define  DEMO_SOCKET_RECEIVE_TASK_PRIO	(DEMO_SOCKET_S_TASK_PRIO + 1)
#define  DEMO_MCAST_TASK_PRIO	(DEMO_SOCKET_RECEIVE_TASK_PRIO + 1)
#define  DEMO_SOCK_FWUP_TASK_PRIO	(DEMO_MCAST_TASK_PRIO + 1)
#define  DEMO_SOCK_S_SEL_TASK_PRIO	(DEMO_SOCK_FWUP_TASK_PRIO + 1)
#define  DEMO_ONESHOT_TASK_PRIO	(DEMO_SOCK_S_SEL_TASK_PRIO + 1)
#define  DEMO_KII_TASK_PRIO	(DEMO_ONESHOT_TASK_PRIO + 1)

#define DEMO_QUEUE_SIZE	32

#define DEMO_BUF_SIZE		TLS_UART_RX_BUF_SIZE

#define DEMO_CONSOLE_CMD			1		//��������cmd
#define DEMO_CONSOLE_SHORT_CMD	2		//CMD��һ���֣�û�н�����

/*����demo�п����õ�����Ϣ*/
#define	DEMO_MSG_WJOIN_FAILD	1
#define	DEMO_MSG_WJOIN_SUCCESS	2
#define DEMO_MSG_SOCKET_RECEIVE_DATA		3
#define	DEMO_MSG_UART_RECEIVE_DATA			4
#define	DEMO_MSG_SOCKET_ERR					5
#define DEMO_MSG_SOCKET_CREATE				6
#define DEMO_MSG_SOCKET_TEST				7
#define DEMO_MSG_OPEN_UART					8


typedef struct demo_sys{
	char *rxbuf;		/*uart rx*/
	char *txbuf;		/*uart tx*/
	u16 wptr;
	u16 rptr;
	u8 overflag;		/*�����־*/
	u8 MsgNum;


	bool socket_ok;
	int socket_num;
	char *sock_rx;
	u16 sock_data_len;

	int recvlen;		//socket���յ����ݳ��ȣ����ڲ���
	int translen;		//socket���͵����ݳ��ȣ����ڲ���
	u8 is_raw; // 1:raw socket; 0:��׼socket
}ST_Demo_Sys;

struct demo_console_info_t {
    char *cmd;
    int (*callfn)(char *buf);	
    char *info;
};


void CreateDemoTask(void);
void DemoStdSockOneshotSendMac(void);
void DemoRawSockOneshotSendMac(void);


#endif
