#include "stm32f4xx.h"
#include "bluetooth.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "MFRC522.h"
#include "oled.h"
#include "as608.h"
#include <string.h>
#include "mqtt.h"

#define  BLE_BUFFERSIZE  256						       //���ջ������Ĵ�С
static __IO char ble_buffer[BLE_BUFFERSIZE];   //��Ϊ����ģ��Ľ��ջ�����
static __IO int  ble_cnt=0;										 //��Ϊ����ģ��Ľ��ռ�����
static __IO uint8_t ble_recvflag = 0;			  	 //��Ϊ����ģ��Ľ��ձ�־λ

__IO char pwd_buf[16]="123456";  //��Ϊ����Ľ��ջ�����  Ĭ������123456
__IO int  pwd_count = 0;				     	   //��Ϊ����Ľ��ռ�����
extern __IO u8 PwdAlarmLimit;            //�����ж�����������ٴ�������ʱ����
__IO char card_id[50][20]={{0}};         //�����洢50�ſ��ţ��������Ϊ50λ
__IO u8 card_cnt = 0;                    //�ſ�¼�������
extern unsigned char BMP1[];
extern volatile uint32_t opencount;
extern volatile uint32_t alarmcount;


//������������
void BLE_Config(u32 baud)
{
	USART3_Config(baud);
	
	Usart_SendString( USART3, "AT+NAMEWGLWGL\r\n");   //�޸���������
	delay_ms(10);
//	USART3_SendString("AT+BAUD4\r\n"); //�������������� 9600
//	delay_ms(10);
	Usart_SendString( USART3, "AT+RESET\r\n"); 				//����������Ч
	delay_ms(10);
}

//�޸�����
void ModPwd(void)
{
	//��ս��ջ�����
	memset((char *)pwd_buf,0,16);
	//������ռ�����
	pwd_count = 0;
	OLED_Clear();
	OLED_ShowCHinese(0,2,99);	   //��������Ҫ�޸ĵ�����
	OLED_ShowCHinese(16,2,104);
	OLED_ShowCHinese(32,2,20);
	OLED_ShowCHinese(48,2,105);
	OLED_ShowCHinese(64,2,106);
	OLED_ShowCHinese(80,2,107);
	OLED_ShowCHinese(96,2,108);
	OLED_ShowCHinese(112,2,109);
	OLED_ShowCHinese(0,4,103);
	OLED_ShowCHinese(16,4,81);
	OLED_ShowString(0,6,"#",16);
	OLED_ShowCHinese(8,6,110); //���
	OLED_ShowCHinese(24,6,111); 
	OLED_ShowString(40,6,"*",111);
	OLED_ShowCHinese(56,6,97); //ȷ��
	OLED_ShowCHinese(72,6,98);
	u8 ClearFlag = 1;
	while(1)
	{
		get_key(); //��ȡ����״̬
		if(KeyState !=' ' && KeyState != '*' && KeyState != '#') //���µļ���Ч ������Ҫ������ִ����Ӧ�Ĳ��� 
		{
			if(ClearFlag) //����
			{
				OLED_Clear();
				ClearFlag=0;
			}
			//������
			if(pwd_count < 16)
			{
				pwd_buf[pwd_count++]=KeyState;
				pwd_buf[pwd_count]='\0';
				OLED_ShowString(8,2,(u8 *)pwd_buf,16);
			}
			// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
			KeyState = ' ';
		}
		else if(KeyState == '*')  //*��ʾȷ�ϼ�
		{
			//����MQTT�ϴ���������
			mqtt_report_devices_status();
			OLED_Clear();
			OLED_ShowCHinese(16,3,103);
			OLED_ShowCHinese(32,3,81);
			OLED_ShowCHinese(48,3,107);
			OLED_ShowCHinese(64,3,108);
			OLED_ShowCHinese(80,3,95);
			OLED_ShowCHinese(96,3,96);
			delay_ms(500); //���ܳ���798
			delay_ms(500); //���ܳ���798
			OLED_Clear();
			OLED_ShowCHinese(0,3,0);	       //С
			OLED_ShowCHinese(18,3,1);	       //��
			OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
			OLED_ShowCHinese(92,3,2);	       //��
			OLED_ShowCHinese(110,3,3);	     //��
			// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
			KeyState = ' ';
			break;
		}
		else if(KeyState == '#')  //#��ʾ�����
		{
			OLED_Clear();
			//��ս��ջ�����
			memset((char *)pwd_buf,0,16);
			//������ռ�����
			pwd_count = 0;
			// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
			KeyState = ' ';
		}	
	}		
}



//����ſ�
void AddKeyCard(void)
{
	u8 status = 1;
	u8 card_pydebuf[2];
	u8 card_numberbuf[5]; //���һ���ֽ���У���ֽ�
	
	OLED_Clear();
	OLED_ShowCHinese(8,3,99);	    //�뿪ʼע���ſ�
	OLED_ShowCHinese(24,3,89);	      
	OLED_ShowCHinese(40,3,100);	       
	OLED_ShowCHinese(56,3,101);	       
	OLED_ShowCHinese(72,3,102);	      
	OLED_ShowCHinese(88,3,2);	       
	OLED_ShowCHinese(104,3,94);	       
	while (1)
	{
		status = MFRC522_Request(0x52, card_pydebuf); //Ѱ��   S50���Ŀ�����ȫ��Ψһ��  ���ڲ�����Ȧ  ���ö�ȡ����ȡ��Ƭ��Ϣ  ��Ƶ 13.56MHZ

		if (status == 0) //���������
		{
			if(card_cnt<50)
			{
				MFRC522_Anticoll(card_numberbuf); //����ײ����  �������ṩ�����Žϴ�Ŀ�Ƭ  ���Եõ�����  ����4�ֽ�
				sprintf((char *)card_id[card_cnt], "0x");
				for (int i = 0; i < 4; i++)
				{
					sprintf((char *)card_id[card_cnt] + 2 + 2 * i, "%02X", card_numberbuf[i]);
				}
				card_cnt++;
				OLED_Clear();
				GPIO_SetBits(GPIOF, GPIO_Pin_8); //��������
				delay_ms(100); //���ܳ���798
				GPIO_ResetBits(GPIOF, GPIO_Pin_8); //��������
				OLED_ShowCHinese(16,2,101);	  //ע���ſ��ɹ�     
				OLED_ShowCHinese(32,2,102);	      
				OLED_ShowCHinese(48,2,2);	       
				OLED_ShowCHinese(64,2,94);
				OLED_ShowCHinese(80,2,95);
				OLED_ShowCHinese(96,2,96);
				OLED_ShowString(24,4,(u8 *)card_id[card_cnt-1],16);
				delay_ms(500); //���ܳ���798
				delay_ms(500); //���ܳ���798
				
				OLED_Clear();
				OLED_ShowCHinese(0,3,0);	       //С
				OLED_ShowCHinese(18,3,1);	       //��
				OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
				OLED_ShowCHinese(92,3,2);	       //��
				OLED_ShowCHinese(110,3,3);	     //��
				status =1;
				break;
			}
			else
			{
				OLED_Clear();
				OLED_ShowString(0,2,"No more than 50 key cards",16);	
				delay_ms(500); //���ܳ���798
				delay_ms(500); //���ܳ���798
				OLED_Clear();
			}
			status =1;
		}
	}
}

//ɾ���ſ�
void DelKeyCard(void)
{
	int index = 0;
	OLED_Clear();
	OLED_ShowCHinese(32,0,54);	   //��˳��ɾ���ſ� 
	OLED_ShowCHinese(48,0,55);
	OLED_ShowCHinese(64,0,2);
	OLED_ShowCHinese(80,0,94);
	OLED_ShowString(24,2,(u8 *)card_id[index],16);
	OLED_ShowString(16,4,"A+B-*",16);
	OLED_ShowCHinese(56,4,97); //ȷ��
	OLED_ShowCHinese(88,4,98);
	while(1)
	{
		get_key(); //��ȡ����״̬
		if(KeyState !=' ' && KeyState == 'A') //���µļ���Ч A��ʾ�Ϸ� 
		{
			if(index < card_cnt-1)
			{
				index++;
				OLED_ShowString(24,2,(u8 *)card_id[index],16);
			}
			// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
			KeyState = ' ';
		}
		else if(KeyState !=' ' && KeyState == 'B')  //B��ʾ�·�
		{
			//������
			if(index >0)
			{
				index--;
				OLED_ShowString(24,2,(u8 *)card_id[index],16);
			}
			// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
			KeyState = ' ';
		}
		else if(KeyState !=' ' && KeyState == '*')  //*��ʾȷ�ϼ�
		{
			if (index >= 0 && index < card_cnt)
			{
				// ��ָ�����������������ǰ�ƶ�һ��λ��
				for (int i = index; i < card_cnt - 1; i++) 
				{
					strcpy((char *)card_id[i], (char *)card_id[i + 1]);
				}
				// �����һ��λ�����
				memset((char *)card_id[card_cnt - 1], 0, sizeof(card_id[card_cnt - 1]));
				// ���¼�����
				card_cnt--;
			}
			OLED_Clear();
			OLED_ShowCHinese(16,2,54);	   //��˳��ɾ���ſ��ɹ�
			OLED_ShowCHinese(32,2,55);
			OLED_ShowCHinese(48,2,2);
			OLED_ShowCHinese(64,2,94);
			OLED_ShowCHinese(80,2,95);
			OLED_ShowCHinese(96,2,96);
			OLED_ShowString(24,4,(u8 *)card_id[index],16);
			delay_ms(500); //���ܳ���798
			OLED_Clear();
			OLED_ShowCHinese(0,3,0);	       //С
			OLED_ShowCHinese(18,3,1);	       //��
			OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
			OLED_ShowCHinese(92,3,2);	       //��
			OLED_ShowCHinese(110,3,3);	     //��
			// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
			KeyState = ' ';
			break;
		}
			
	}		
}

//�ж������Ƿ���յ����ݲ����д���
void BlueRecvProcessing(void)
{
	if( 1 == ble_recvflag )
	{
		
		//������ջ�����
		if(strstr((char *)ble_buffer,"OpenDoor#")!=NULL)
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
			GPIO_SetBits(GPIOF, GPIO_Pin_15); //�ſ�
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
			GPIO_ResetBits(GPIOF, GPIO_Pin_15); //�Ź�
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
			OLED_Clear(); 	//����
			OLED_ShowCHinese(0,3,0);	       //С
			OLED_ShowCHinese(18,3,1);	       //��
			OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
			OLED_ShowCHinese(92,3,2);	       //��
			OLED_ShowCHinese(110,3,3);	     //��
		}
		else if(strstr((char *)ble_buffer,"ModPwd#")!=NULL)
		{
			ModPwd(); //�޸����뺯��
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
		}
		else if(strstr((char *)ble_buffer,"AddKeyCard#")!=NULL)
		{
			//����ſ�
			AddKeyCard();
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
		}
		else if(strstr((char *)ble_buffer,"DelKeyCard#")!=NULL)
		{
			//ɾ���ſ�
			DelKeyCard();
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
		}
		else if(strstr((char *)ble_buffer,"Add_FR#")!=NULL)
		{ 
			//¼ָ��
			Add_FR();
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
			OLED_Clear(); 	//����
			OLED_ShowCHinese(0,3,0);	       //С
			OLED_ShowCHinese(18,3,1);	       //��
			OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
			OLED_ShowCHinese(92,3,2);	       //��
			OLED_ShowCHinese(110,3,3);	     //��
			
		}
		else if(strstr((char *)ble_buffer,"Del_FR#")!=NULL)
		{ 
			//ɾ��ָ��
			Del_FR();
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
			OLED_Clear(); 	//����
			OLED_ShowCHinese(0,3,0);	       //С
			OLED_ShowCHinese(18,3,1);	       //��
			OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
			OLED_ShowCHinese(92,3,2);	       //��
			OLED_ShowCHinese(110,3,3);	     //��
			
		}
		else
		{
			PwdAlarmLimit++;
			if(PwdAlarmLimit==8) //����������������10�ξͱ���
			{
				alarmcount++;
				//����MQTT�ϴ���������
				mqtt_report_devices_status();
				for(int i=0;i<25;i++)
				{
					GPIO_SetBits(GPIOF, GPIO_Pin_8); //��������
					delay_ms(50); //���ܳ���798
					GPIO_ResetBits(GPIOF, GPIO_Pin_8); //��������
					delay_ms(50); //���ܳ���798
				}
				PwdAlarmLimit=0; //��λ����ֹ���󱨾�
			}
		}
			
		
		//������ձ�־λ
		ble_recvflag = 0;
		
		//��ս��ջ�����
		memset((char *)ble_buffer,0,BLE_BUFFERSIZE);
		
		//������ռ�����
		ble_cnt = 0;
		
	}
}

//�жϷ����� ������������
void USART3_IRQHandler(void)
{
	
	//�ж��ж��Ƿ񴥷�
	if( USART_GetITStatus(USART3, USART_IT_RXNE) == SET )
	{
		//�жϽ��ջ������Ƿ�����
		if(ble_cnt < BLE_BUFFERSIZE)
		{
			ble_buffer[ble_cnt++] = USART_ReceiveData(USART3); //һ��ֻ�ܽ���1���ֽ�
			
			//˵�����ݽ������
			if(ble_buffer[ble_cnt-1] == '#')
			{
				ble_recvflag = 1;
			}
		}
		
	}
}



