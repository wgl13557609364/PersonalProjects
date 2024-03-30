#include "esp8266.h"

//����ȫ�ֱ���
volatile uint32_t esp8266_transparent_transmission_sta=0;

//����ͨ�Ŵ���
void esp8266_init(void)
{
	UART4_Config(115200);
}

/* ����WiFi�����ȵ� */
int32_t Esp8266_Init(void)
{
	int32_t ret;
	
	//esp8266_wifi���ô���3ͨ��,ǰ�������ô���3
	//esp8266_init();

	//�˳�͸��ģʽ����������ATָ��
	ret=esp8266_exit_transparent_transmission();
	if(ret)
	{
		printf("esp8266_exit_transparent_transmission fail\r\n");
		return -1;
	}	
	printf("esp8266_exit_transparent_transmission success\r\n");
	delay_ms(500);
	delay_ms(500);
	//��λģ��
	ret=esp8266_reset();
	if(ret)
	{
		printf("esp8266_reset fail\r\n");
		return -2;
	}
	printf("esp8266_reset success\r\n");
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	
	//���ESP8266�Ƿ�����
	ret=esp8266_check();
	if(ret)
	{
		printf("esp8266_check fail\r\n");
		return -3;
	}
	printf("esp8266_check success\r\n");
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	
	//�رջ���
	ret=esp8266_enable_echo(0);
	if(ret)
	{
		printf("esp8266_enable_echo(0) fail\r\n");
		return -4;
	}	
	printf("esp8266_enable_echo(0)success\r\n");
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);	

	//�����ȵ�
	ret = esp8266_connect_ap(WIFI_SSID,WIFI_PASSWORD);
	if(ret)
	{
		printf("esp8266_connect_ap fail\r\n");
		return -5;
	}	
	printf("esp8266_connect_ap success\r\n");
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);

	return 0;
}

//����ATָ��
void esp8266_send_at(char *str)
{
	//��ս��ջ�����
	memset((void *)Rx4Buffer,0, sizeof Rx4Buffer);
	
	//��ս��ռ���ֵ
	Rx4Counter = 0;	
	
	//����3��������
	Usart_SendString(UART4, str);
}

//�����ֽ�
void esp8266_send_bytes(uint8_t *buf,uint8_t len)
{
	Usart_SendBytes(UART4, buf,len);
}

//�����ַ���
void esp8266_send_str(char *buf)
{
	Usart_SendString(UART4, buf);
}

/* ���ҽ������ݰ��е��ַ��� */
int32_t esp8266_find_str_in_rx_packet(char *str,uint32_t timeout)
{
	char *dest = str;
	char *src  = (char *)&Rx4Buffer;
	
	//�ȴ����ڽ�����ϻ�ʱ�˳���strstr()Ѱ����Ӧ�ַ��������δ�ҵ��򷵻� Null;
	while((strstr(src,dest)==NULL) && timeout) //while(�ҵ��� ��=  NULL && timeout == 0),�˳�ѭ����
	{		
		delay_ms(1);
		timeout--;
	}

	if(timeout) 
		return 0;	//���ҵ����������
		                    
	return -1; 
}

/* ���ESP8266�Ƿ����� */
int32_t esp8266_check(void)
{
	esp8266_send_at("AT\r\n");
	
	if(esp8266_find_str_in_rx_packet("OK",10000))
		return -1;

	return 0;
}

/* ��λ */
int32_t esp8266_reset(void)
{
	esp8266_send_at("AT+RST\r\n");
	
	if(esp8266_find_str_in_rx_packet("OK",10000))
		return -1;

	return 0;
}

/* ���Դ򿪻�ر� */
int32_t esp8266_enable_echo(uint32_t b)
{
	if(b)
		esp8266_send_at("ATE1\r\n"); 
	else
		esp8266_send_at("ATE0\r\n"); 
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;

	return 0;
}

/* �˳�͸��ģʽ */
int32_t esp8266_exit_transparent_transmission (void)
{
	esp8266_send_at ("+++");
	
	//�˳�͸��ģʽ��������һ��ATָ��Ҫ���1��
	delay_ms(1000); 
	
	//��¼��ǰesp8266�����ڷ�͸��ģʽ
	esp8266_transparent_transmission_sta = 0;

	return 0;
}

/* ����͸��ģʽ */
int32_t esp8266_entry_transparent_transmission(void)
{
	//����͸��ģʽ
	esp8266_send_at("AT+CIPMODE=1\r\n");  
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	delay_ms(1000);delay_ms(1000);

	//��������״̬
	esp8266_send_at("AT+CIPSEND\r\n");
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -2;
	delay_ms(1000);delay_ms(1000);

	//��¼��ǰesp8266������͸��ģʽ
	esp8266_transparent_transmission_sta = 1;
	return 0;
}

/* �������״̬ */
int32_t esp8266_check_connection_status(void)
{
	esp8266_send_at("AT+CIPSTATUS\r\n");
	
	if(esp8266_find_str_in_rx_packet("STATUS:3",10000))
		if(esp8266_find_str_in_rx_packet("OK",10000))
			return -1;

	return 0;
}

/**
 * ���ܣ������ȵ�
 * ������
 *         ssid:�ȵ���
 *         pwd:�ȵ�����
 * ����ֵ��
 *         ���ӽ��,��0���ӳɹ�,0����ʧ��
 * ˵���� 
 *         ʧ�ܵ�ԭ�������¼���(UARTͨ�ź�ESP8266���������)
 *         1. WIFI�������벻��ȷ
 *         2. ·���������豸̫��,δ�ܸ�ESP8266����IP
 */
int32_t esp8266_connect_ap(char* ssid,char* pswd)
{
#if 0	
	//������ʹ������sprintf��ռ�ù����ջ
	char buf[128]={0};
	
	//����ΪSTATIONģʽ	
	esp8266_send_at("AT+CWMODE_CUR=1\r\n"); 
	if(esp8266_find_str_in_rx_packet("OK",10000))
		return -1;
	esp8266_send_at("AT+CIPMUX=0\r\n");
	if(esp8266_find_str_in_rx_packet("OK",1000))
		return -2;
	sprintf(buf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);
	esp8266_send_at(buf); 
	if(esp8266_find_str_in_rx_packet("OK",5000))
		if(esp8266_find_str_in_rx_packet("CONNECT",5000))
			return -2;
#else
	//����ΪStationģʽ	
	esp8266_send_at("AT+CWMODE_CUR=1\r\n"); 
	if(esp8266_find_str_in_rx_packet("OK",1000))
		return -1;

	esp8266_send_at("AT+CIPMUX=0\r\n");
	if(esp8266_find_str_in_rx_packet("OK",1000))
		return -2;

	//�����ȵ�
	esp8266_send_at("AT+CWJAP_CUR="); 
	esp8266_send_at("\"");esp8266_send_at(ssid);esp8266_send_at("\"");	
	esp8266_send_at(",");	
	esp8266_send_at("\"");esp8266_send_at(pswd);esp8266_send_at("\"");	
	esp8266_send_at("\r\n");
	//�����ȵ㣬��صȴ�����ָ���WIFI GOT IP����ʾ���ӳɹ����ٷ��������ָ�
	while(esp8266_find_str_in_rx_packet("WIFI GOT IP",5000));
#endif
	return 0;
}

/**
 * ���ܣ�ʹ��ָ��Э��(TCP/UDP)���ӵ�������
 * ������
 *         mode:Э������ "TCP","UDP"
 *         ip:Ŀ�������IP
 *         port:Ŀ���Ƿ������˿ں�
 * ����ֵ��
 *         ���ӽ��,��0���ӳɹ�,0����ʧ��
 * ˵���� 
 *         ʧ�ܵ�ԭ�������¼���(UARTͨ�ź�ESP8266���������)
 *         1. Զ�̷�����IP�Ͷ˿ں�����
 *         2. δ����AP
 *         3. �������˽�ֹ���(һ�㲻�ᷢ��)
 */
int32_t esp8266_connect_server(char* mode,char* ip,uint16_t port)
{

#if 0	
	//ʹ��MQTT���ݵ�ip��ַ������������ʹ�����·�����������ջ���
	//AT+CIPSTART="TCP","a10tC4OAAPc.iot-as-mqtt.cn-shanghai.aliyuncs.com",1883�����ַ���ռ���ڴ������
	
	char buf[128]={0};
	
	//���ӷ�����
	sprintf((char*)buf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
	
	esp8266_send_at(buf);
#else
	char buf[16]={0};
	esp8266_send_at("AT+CIPSTART=");
	esp8266_send_at("\"");	esp8266_send_at(mode);	esp8266_send_at("\"");
	esp8266_send_at(",");
	esp8266_send_at("\"");	esp8266_send_at(ip);	esp8266_send_at("\"");	
	esp8266_send_at(",");
	sprintf(buf,"%d",port);
	esp8266_send_at(buf);	
	esp8266_send_at("\r\n");
#endif
	
	if(esp8266_find_str_in_rx_packet("CONNECT",5000))
		if(esp8266_find_str_in_rx_packet("OK",5000))
			return -1;
	return 0;
}

/* �Ͽ������� */
int32_t esp8266_disconnect_server(void)
{
	esp8266_send_at("AT+CIPCLOSE\r\n");
		
	if(esp8266_find_str_in_rx_packet("CLOSED",2000))
		if(esp8266_find_str_in_rx_packet("OK",2000))
			return -1;

	return 0;	
}

/* ʹ�ܶ����� */
int32_t esp8266_enable_multiple_id(uint32_t b)
{
	char buf[32]={0};
	
	sprintf(buf,"AT+CIPMUX=%d\r\n", b);
	esp8266_send_at(buf);
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	return 0;
}

/* ���������� */
int32_t esp8266_create_server(uint16_t port)
{
	char buf[32]={0};
	
	sprintf(buf,"AT+CIPSERVER=1,%d\r\n", port);
	esp8266_send_at(buf);
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	return 0;
}

/* �رշ����� */
int32_t esp8266_close_server(uint16_t port)
{
	char buf[32]={0};
	
	sprintf(buf,"AT+CIPSERVER=0,%d\r\n", port);
	esp8266_send_at(buf);
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	return 0;
}


/*
����Ϊ�򵥿������Esp8266_WiFiģ��ΪAPģʽ���й�����������AP������
*/
void Wifi_Init(void)
{
	UART4_sendstr(UART4,"AT+CWMODE=2\r\n");//����Ϊ softAP+station ����ģʽ
	delay_ms(500);
	
	UART4_sendstr(UART4,"AT+RST\r\n");//����
	delay_ms(1500);
	
	UART4_sendstr(UART4,"AT+CIPAP=\"192.168.1.1\"\r\n");//����IP:192.168.1.1
	delay_ms(500);

	UART4_sendstr(UART4,"AT+CWSAP=\"CZJ\",\"12345678\",5,0\r\n");//����wifi������CZJ������12345678�����5����ͬʱ���ӣ�����ʱ�������룻
	delay_ms(500);
	
	UART4_sendstr(UART4,"AT+CIPMUX=1\r\n");//����������
	delay_ms(500);
	
	UART4_sendstr(UART4,"AT+CIPSERVER=1,8080\r\n");//���ö˿�8080
	delay_ms(500);
	
	printf("wifi_init end\n");//����1�����ʾ��
}
//wifiģ�鷢�����---ÿ�ι̶�����num���ֽ�
void wifisend(char *buf,int num)
{
	//ÿ��wifiģ�鷢�����ݵ�ʱ�򣬶����ȷ���һ���̶�ǰ׺
	char sendfront[32];//����ǰ׺�Ļ�����
	sprintf(sendfront,"AT+CIPSEND=0,%d\r\n",num);//����ַ���
	
	UART4_sendstr(UART4,sendfront);
	delay_ms(5);
	UART4_sendstr(UART4,buf);
}
//����len���ȵ��ַ���
void UART4_sendlenth(USART_TypeDef* USARTx, uint8_t *Data,uint8_t Len)
{ 
	while(Len--){				                          //�ж��Ƿ񵽴��ַ���������
	    USART_SendData(USARTx, *Data++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET); //�ȴ�������
	}
}
//����һ���������ַ���
void UART4_sendstr(USART_TypeDef* USARTx, char *Data)
{ 
	//ѭ������1���ֽڣ�ֱ��׼�����͵��ֽ���'\0',Ҳ�����ַ���ĩβ��ֹͣ����
	while(*Data!=0){				                        
		USART_SendData(USARTx, *Data);
		Data++;
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}
