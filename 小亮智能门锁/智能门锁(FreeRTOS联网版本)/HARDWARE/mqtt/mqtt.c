#include "mqtt.h"
#include "stdbool.h"
#include "cJSON.h"
#include "oled.h"
char			mqtt_post_msg[526];
uint32_t		mqtt_tx_len;
const uint8_t 	g_packet_heart_reply[2] = {0xc0,0x00};

extern __IO char pwd_buf[16];
extern volatile uint32_t opencount;
extern volatile uint32_t alarmcount;
extern unsigned char BMP1[];

//����MQTT���Ӱ�����
int Mqtt_Connect_Aliyun(void)
{
	int ret = 0;
	//���ӵ�Ŀ��TCP������
	ret =esp8266_connect_server("TCP",MQTT_BROKERADDRESS_DEVICE1,1883);
	if(ret)
	{
		printf("esp8266_connect_server fail\r\n");
		return -5;
	}	
	printf("esp8266_connect_server success\r\n");
	delay_ms(300);
	
	//�������״̬
	ret = esp8266_check_connection_status();
	if(ret)
	{
		printf("esp8266_check_connection_status fail\r\n");
		
		//���������ȵ�
		while(Esp8266_Init());
	}
	printf("esp8266_check_connection_status success\r\n");
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	
	//����͸��ģʽ
	ret = esp8266_entry_transparent_transmission();
	if(ret)
	{
		printf("esp8266_entry_transparent_transmission fail\r\n");
		return -6;
	}	
	printf("esp8266_entry_transparent_transmission success\r\n");
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	delay_ms(500);
	
	//MQTT�����ƶ�
	if(mqtt_connect(MQTT_CLIENTID_DEVICE1, MQTT_USARNAME_DEVICE1, MQTT_PASSWD_DEVICE1))
	{
		printf("mqtt_connect DEVICE1 fail\r\n");
		return -7;	
	
	}
	
	printf("mqtt_connect success\r\n");
	delay_ms(500);
	delay_ms(500);
	
	
	//MQTT��������
	if(mqtt_subscribe_topic(MQTT_SUBSCRIBE_TOPIC_DEVICE1,0,1))
	{
		printf("mqtt_subscribe_topic DEVICE1 fail\r\n");
		return -8;
	}	
	
	
	printf("mqtt_subscribe_topic success\r\n");

	return 0;
}

//MQTT���ӷ������Ĵ������
int32_t mqtt_connect(char *client_id,char *user_name,char *password)
{
	uint8_t encodedByte = 0;
    uint32_t client_id_len = strlen(client_id);
    uint32_t user_name_len = strlen(user_name);
    uint32_t password_len  = strlen(password);
    uint32_t data_len;
    uint32_t cnt =2;
    uint32_t wait=0;
    mqtt_tx_len  =0;
	
    //�ɱ䱨ͷ+Payload  ÿ���ֶΰ��������ֽڵĳ��ȱ�ʶ
    data_len = 10 + (client_id_len+2) + (user_name_len+2) + (password_len+2);

    //�̶���ͷ
    //���Ʊ�������
    Tx4Buffer[mqtt_tx_len++] = 0x10;			//MQTT Message Type CONNECT
    //ʣ�೤��(�������̶�ͷ��)
    do
    {
        encodedByte = data_len % 128;
        data_len = data_len / 128;
        //if there are more data to encode, set the top bit of this byte
        if( data_len > 0 )
            encodedByte = encodedByte | 128;
        Tx4Buffer[mqtt_tx_len++] = encodedByte;
    } while( data_len > 0 );

    //�ɱ䱨ͷ
    //Э����
    Tx4Buffer[mqtt_tx_len++] = 0;			// Protocol Name Length MSB
    Tx4Buffer[mqtt_tx_len++] = 4;			// Protocol Name Length LSB
    Tx4Buffer[mqtt_tx_len++] = 'M';			// ASCII Code for M
    Tx4Buffer[mqtt_tx_len++] = 'Q';			// ASCII Code for Q
    Tx4Buffer[mqtt_tx_len++] = 'T';			// ASCII Code for T
    Tx4Buffer[mqtt_tx_len++] = 'T';			// ASCII Code for T
    //Э�鼶��
    Tx4Buffer[mqtt_tx_len++] = 4;			// MQTT Protocol version = 4
    //���ӱ�־
    Tx4Buffer[mqtt_tx_len++] = 0xc2;		// conn flags
    Tx4Buffer[mqtt_tx_len++] = 0;			// Keep-alive Time Length MSB
    Tx4Buffer[mqtt_tx_len++] = 250;			// Keep-alive Time Length LSB  250S������

    Tx4Buffer[mqtt_tx_len++] = BYTE1(client_id_len);// Client ID length MSB
    Tx4Buffer[mqtt_tx_len++] = BYTE0(client_id_len);// Client ID length LSB
    memcpy(&Tx4Buffer[mqtt_tx_len],client_id,client_id_len);
    mqtt_tx_len += client_id_len;

    if(user_name_len > 0)
    {
        Tx4Buffer[mqtt_tx_len++] = BYTE1(user_name_len);		//user_name length MSB
        Tx4Buffer[mqtt_tx_len++] = BYTE0(user_name_len);		//user_name length LSB
        memcpy(&Tx4Buffer[mqtt_tx_len],user_name,user_name_len);
        mqtt_tx_len += user_name_len;
    }

    if(password_len > 0)
    {
        Tx4Buffer[mqtt_tx_len++] = BYTE1(password_len);			//password length MSB
        Tx4Buffer[mqtt_tx_len++] = BYTE0(password_len);			//password length LSB
        memcpy(&Tx4Buffer[mqtt_tx_len],password,password_len);
        mqtt_tx_len += password_len;
    }

    while(cnt--)
    {
        memset((void *)Rx4Buffer,0,sizeof(Rx4Buffer));
		Rx4Counter=0;
		
        mqtt_send_bytes(Tx4Buffer,mqtt_tx_len);
		
		//�ȴ�3sʱ��
        wait=3000;
		
        while(wait--)
        {
			delay_ms(1);

			//�������ȷ�Ϲ̶���ͷ
            if((Rx4Buffer[0]==0x20) && (Rx4Buffer[1]==0x02))
            {
				if(Rx4Buffer[3] == 0x00)
				{
					printf("�����ѱ��������˽��ܣ�����ȷ�ϳɹ�\r\n");
					//���ӳɹ�
					return 0;
				}
				else
				{
					switch(Rx4Buffer[3])
					{
						case 1:printf("�����Ѿܾ�����֧�ֵ�Э��汾\r\n");
						break;
						case 2:printf("�����Ѿܾ������ϸ�Ŀͻ��˱�ʶ��\r\n");
						break;		
						case 3:printf("�����Ѿܾ�������˲�����\r\n");
						break;		
						case 4:printf("�����Ѿܾ�����Ч���û�������\r\n");
						break;	
						case 5:printf("�����Ѿܾ���δ��Ȩ\r\n");
						break;
						default:printf("δ֪��Ӧ\r\n");
						break;
					}
					return 0;
				} 
            }  
        }
    }
	
    return -1;
}

/**
  * @brief  MQTT����/ȡ���������ݴ������
  * @param  topic  		����
  * @param  qos    		��Ϣ�ȼ�
  * @param  whether: 	����/ȡ�����������
  * @retval 0���ɹ���
  * 		1��ʧ�ܣ�
  */
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{
	uint8_t encodedByte=0;
    uint32_t cnt=2;
    uint32_t wait=0;
	
    uint32_t topiclen = strlen(topic);
    uint32_t data_len = 2 + (topiclen+2) + (whether?1:0);//�ɱ䱨ͷ�ĳ��ȣ�2�ֽڣ�������Ч�غɵĳ���
	
	mqtt_tx_len=0;
	
    //�̶���ͷ
    //���Ʊ�������
    if(whether) 
		Tx4Buffer[mqtt_tx_len++] = 0x82; 	//��Ϣ���ͺͱ�־����
    else	
		Tx4Buffer[mqtt_tx_len++] = 0xA2; 	//ȡ������

    //ʣ�೤��
    do
    {
        encodedByte = data_len % 128;
        data_len 	= data_len / 128;
        //if there are more data to encode, set the top bit of this byte
        if ( data_len > 0 )
            encodedByte = encodedByte | 128;
        Tx4Buffer[mqtt_tx_len++] = encodedByte;
    } while ( data_len > 0 );

    //�ɱ䱨ͷ
    Tx4Buffer[mqtt_tx_len++] = 0;				//��Ϣ��ʶ�� MSB
    Tx4Buffer[mqtt_tx_len++] = 0x01;			//��Ϣ��ʶ�� LSB
	
    //��Ч�غ�
    Tx4Buffer[mqtt_tx_len++] = BYTE1(topiclen);	//���ⳤ�� MSB
    Tx4Buffer[mqtt_tx_len++] = BYTE0(topiclen);	//���ⳤ�� LSB
    memcpy(&Tx4Buffer[mqtt_tx_len],topic,topiclen);
	
    mqtt_tx_len += topiclen;

    if(whether)
    {
        Tx4Buffer[mqtt_tx_len++] = qos;			//QoS����
    }

    while(cnt--)
    {
		Rx4Counter=0;
        memset((void *)Rx4Buffer,0,sizeof(Rx4Buffer));
        mqtt_send_bytes(Tx4Buffer,mqtt_tx_len);
		
        wait=3000;								//�ȴ�3sʱ��
        while(wait--)
        {
			delay_ms(1);
			
			//��鶩��ȷ�ϱ�ͷ
            if(Rx4Buffer[0]==0x90)
            {
				printf("��������ȷ�ϳɹ�\r\n");
				
				//��ȡʣ�೤��
				if(Rx4Buffer[1]==3)
				{
					printf("Success - Maximum QoS 0 is %02X\r\n",Rx4Buffer[2]);
					printf("Success - Maximum QoS 2 is %02X\r\n",Rx4Buffer[3]);		
					printf("Failure is %02X\r\n",Rx4Buffer[4]);	
				}
				//��ȡʣ�೤��
				if(Rx4Buffer[1]==2)
				{
					printf("Success - Maximum QoS 0 is %02X\r\n",Rx4Buffer[2]);
					printf("Success - Maximum QoS 2 is %02X\r\n",Rx4Buffer[3]);			
				}				
				
				//��ȡʣ�೤��
				if(Rx4Buffer[1]==1)
				{
					printf("Success - Maximum QoS 0 is %02X\r\n",Rx4Buffer[2]);		
				}
				
				//���ĳɹ�
                return 0;
            }
        }
    }
	
    if(cnt) 
		return 0;//���ĳɹ�
	
    return -1;
}

/**
  * @brief  MQTT����/ȡ���������ݴ������
  * @param  topic  		����
  * @param  message  	��Ϣ
  * @param  qos    		��Ϣ�ȼ�
  * @retval 0���ɹ���
  * 		1��ʧ�ܣ�
  */
uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{
//static 
	uint16_t id=0;	
    uint32_t topicLength = strlen(topic);
    uint32_t messageLength = strlen(message);

    uint32_t data_len;
	uint8_t encodedByte;

    mqtt_tx_len=0;
    //��Ч�غɵĳ����������㣺�ù̶���ͷ�е�ʣ�೤���ֶε�ֵ��ȥ�ɱ䱨ͷ�ĳ���
    //QOSΪ0ʱû�б�ʶ��
    //���ݳ���             ������   ���ı�ʶ��   ��Ч�غ�
    if(qos)	data_len = (2+topicLength) + 2 + messageLength;
    else	data_len = (2+topicLength) + messageLength;

		//��ջ�����
		memset(Tx4Buffer,0,512);
	
    //�̶���ͷ
    //���Ʊ�������
    Tx4Buffer[mqtt_tx_len++] = 0x30;				// MQTT Message Type PUBLISH

    //ʣ�೤��
    do
    {
        encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( data_len > 0 )
            encodedByte = encodedByte | 128;
        Tx4Buffer[mqtt_tx_len++] = encodedByte;
    } while ( data_len > 0 );

    Tx4Buffer[mqtt_tx_len++] = BYTE1(topicLength);	//���ⳤ��MSB
    Tx4Buffer[mqtt_tx_len++] = BYTE0(topicLength);	//���ⳤ��LSB
	
    memcpy(&Tx4Buffer[mqtt_tx_len],topic,topicLength);	//��������
	
    mqtt_tx_len += topicLength;

    //���ı�ʶ��
    if(qos)
    {
        Tx4Buffer[mqtt_tx_len++] = BYTE1(id);
        Tx4Buffer[mqtt_tx_len++] = BYTE0(id);
        id++;
    }
	
    memcpy(&Tx4Buffer[mqtt_tx_len],message,messageLength);
	
    mqtt_tx_len += messageLength;
	

	mqtt_send_bytes(Tx4Buffer,mqtt_tx_len);
		
	//��ջ�����
	memset(Tx4Buffer,0,512);
		
	//Qos�ȼ����õ���00����˰�����������ƽ̨��û�з�����Ӧ��Ϣ��;
	return mqtt_tx_len;
}

//�豸״̬�ϱ�
void mqtt_report_devices_status(void)
{

    //�ѿ�������ص�״̬��������sprintf������ŵ�һ��������ٰѸ���������MQTTЭ��������Ϣ����
    //����ʵ��ƽ̨���ݶ�Ӧ���豸��Ϣ������������Ϣ��
	sprintf(mqtt_post_msg,
		"{\"method\":\"thing.service.property.set\",\"id\":\"123456789\",\"params\":{\"doorpassword\":\"%s\",\"alarmcount\":%d,\"opencount\":%d\
		},\"version\":\"1.0.0\"}",
		pwd_buf,alarmcount,opencount);

	//�ϱ���Ϣ��ƽ̨������
	mqtt_publish_data(MQTT_PUBLISH_TOPIC_DEVICE1,mqtt_post_msg,0);
	printf("messge publish to aliyun server OK\r\n");
	
	memset(mqtt_post_msg,0,526);//���㻺����
}

//MQTT��������
void mqtt_send_bytes(uint8_t *buf,uint32_t len)
{
    esp8266_send_bytes(buf,len);
}

//����������
int32_t mqtt_send_heart(void)
{	
	uint8_t buf[2]={0xC0,0x00};
//	int32_t cnt=2;
//	int32_t wait=0;	
	
#if 1	
	mqtt_send_bytes(buf,2);
	printf("�������ѷ��͡�\r\n");
	return 0;
#else
	while(cnt--)
	{	
		memset((void *)Rx4Buffer,0,sizeof(Rx4Buffer));
		Rx4Counter=0;
		Rx4End=0;
		mqtt_send_bytes(buf,2);
		wait=2000;//�ȴ�2sʱ��
		
		while(wait--)
		{
			delay_ms(1);

			//���������Ӧ�̶���ͷ
			if((Rx4Buffer[0]==0xD0) && (Rx4Buffer[1]==0x00)) 
			{
				printf("������Ӧȷ�ϳɹ������������ߡ�\r\n");
				//���ڽ�������λ����0,���������
				Rx4Counter = 0;	
				memset((void *)Rx4Buffer,0,sizeof(Rx4Buffer));				
				Rx4End=0;
				return 0;
			}
		}
	}
	printf("������Ӧȷ��ʧ�ܣ�����������\r\n");
	//���ڽ�������λ����0,���������
	Rx4Counter = 0;	
	memset((void *)Rx4Buffer,0,sizeof(Rx4Buffer));	
	Rx4End=0;
	return -1;
#endif	

}

//MQTT�������Ͽ�
void mqtt_disconnect(void)
{
	uint8_t buf[2]={0xe0,0x00};
	
    mqtt_send_bytes(buf,2);
	
	esp8266_disconnect_server();
}


/*
����mqttЭ�鷢����Ϣ���ݰ� = 0x30+ʣ�೤��+01+00+Topic������+Json���ݣ�����ͨ��������������ƽ̨��������			
0x30 0xE2 0x01 0x00 /thing/service/property/set{"method":"thing.service.property.set","id":"1597870845","params":{"NO":1,"led1":1,"led2":1},"version":"1.0.0"}		
����cJSONʱ����ȫΪ�ַ�����������0x00����������0x00�ᵼ��ֱ�ӽ���cJSON�ġ������Ҫ���в���'{'��ͷ��Json����				
*/
//���������·�����
void mqtt_msg_handle(void)
{
	uint8_t i;
	//���ڽ�������λ����0,���������
//	Rx4Counter = 0;			
//	Rx4End=0;
//	memset((char *)Rx4Buffer,0,512);
	
	//���˴��ɲ����������������յ��������·��������Ƿ�����С�"method"�������ǣ���������ַ����а���\0�Ļ���strstr������ҵ����ͻ᷵��null����
	//if( ! esp8266_find_str_in_rx_packet("method",5000));
	{
 		for(i=0;i<Rx4Counter;i++)
		{	
			//����'{'
			if(Rx4Buffer[i] == '{')
			{
				//�����ɹ������˳�
				if( ! mqtt_cjson_parse((char *)&Rx4Buffer[i]))
					break;
			}
		}
	delay_ms(50); //�ȴ����ݽ������
		
		//���ڽ�������λ����0,���������
	Rx4Counter = 0;			
	Rx4End=0;
	memset((char *)Rx4Buffer,0,512);
		
	}
}

//����MQTT�·�����
/*{
    "method":"thing.service.property.set",
    "id":"1597870845",
    "params":{
        "NO":1,
        "led1":1,
        "led2":1
    },
    "version":"1.0.0"
}
*/

#if 1
uint32_t mqtt_cjson_parse(char *pbuf)
{
	cJSON *Json=NULL, *Method=NULL, *Id=NULL, *Params=NULL, *Item=NULL;

	char *psrt = pbuf;

	//�������ݰ�
	Json = cJSON_Parse(psrt);
	if(Json == NULL)												//���Json���ݰ��Ƿ�����﷨�ϵĴ��󣬷���NULL��ʾ���ݰ���Ч
	{
		cJSON_Delete(Json);

		//��ӡ���ݰ��﷨�����λ��
		printf("Error before: [%s] \r\n", cJSON_GetErrorPtr());
		return 1;
	}
	else
	{
		//ƥ���Ӷ��� method
		if((Method = cJSON_GetObjectItem(Json,"method")) != NULL)
		{
			printf("---------------------------------method----------------------------\r\n");
			printf("%s: %s \r\n",Method->string,Method->valuestring);
		}
		//ƥ���Ӷ��� id
		if((Id = cJSON_GetObjectItem(Json,"id")) != NULL)
		{
			printf("-----------------------------------id------------------------------\r\n");
			printf("%s: %s \r\n",Id->string,Id->valuestring);
		}

		//ƥ���Ӷ��� params
		if((Params = cJSON_GetObjectItem(Json,"params")) != NULL)
		{
			printf("---------------------------------params----------------------------\r\n");

			if((Item = cJSON_GetObjectItem(Params,"openbtn")) != NULL)//ƥ���Ӷ���2�еĳ�Ա "led1"
			{
				printf("%s: %d \r\n",Item->string,Item->valueint);
				if(Item->valueint)
				{
					OLED_Clear(); 	//����
					OLED_ShowCHinese(32,2,2);	   //��˳�����Ѵ� 
					OLED_ShowCHinese(48,2,87);
					OLED_ShowCHinese(64,2,88);
					OLED_ShowCHinese(80,2,89);
					OLED_ShowCHinese(32,4,90);	   //��˳�򣺻�ӭ���� 
					OLED_ShowCHinese(48,4,91);
					OLED_ShowCHinese(64,4,92);
					OLED_ShowCHinese(80,4,93);
					GPIO_SetBits(GPIOF, GPIO_Pin_15);//�ſ�
					opencount++;
					//����MQTT�ϴ���������
					mqtt_report_devices_status();
					GPIO_SetBits(GPIOF, GPIO_Pin_8); //��������
					delay_ms(200); //���ܳ���798
					GPIO_ResetBits(GPIOF, GPIO_Pin_8); //��������
					for(int i = 0 ; i<=8 ; i++)
					{
						delay_ms(500); //���ܳ���798
					}
					GPIO_ResetBits(GPIOF, GPIO_Pin_15);//�Ź�
					OLED_Clear(); 	//����
					OLED_ShowCHinese(0,3,0);	       //С
					OLED_ShowCHinese(18,3,1);	       //��
					OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
					OLED_ShowCHinese(92,3,2);	       //��
					OLED_ShowCHinese(110,3,3);	     //��
				}
			}
			
		}
	}

	//�ͷ�cJSON_Parse()����������ڴ�ռ�
	cJSON_Delete(Json);
	Json=NULL;
	
	return 0;
}
#endif
