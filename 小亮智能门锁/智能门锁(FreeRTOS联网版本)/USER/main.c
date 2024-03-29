#include "stm32f4xx.h"
#include "bluetooth.h"
#include "MFRC522.h"
#include "Relay.h"
#include "oled.h"
#include "beep.h"
#include "key.h"
#include "timer.h"
#include "as608.h"
#include "esp8266.h"
#include "mqtt.h"
#include "usart.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "bmp.h"
#include "stm32f4xx.h"  //�������
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"		  //�û����з�װ����ʱ����������ʵ�־�ȷ�ķ�������ʱ

__IO char PwdIn_Buf[16] = {0}; //�洢��������
__IO int PwdIn_cnt = 0;        //�������������
__IO u8 PwdAlarmLimit = 0;     //�����ж�����������ٴ�������ʱ����
__IO u8 status=1;
__IO u8 card_pydebuf[2];
__IO u8 card_numberbuf[5];  //���һ���ֽ���У���ֽ�
volatile uint32_t opencount=0; //���Ŵ���
volatile uint32_t alarmcount = 0; //��������
extern volatile int timeflag;

void PasswordInput(void)
{
	if(KeyState !=' ' && KeyState != '*' && KeyState != '#') //���µļ���Ч ������Ҫ������ִ����Ӧ�Ĳ��� 
	{
		OLED_Clear();
		u8 ClearFlag = 0; //������־
		//��ս��ջ�����
		memset((char *)PwdIn_Buf,0,16);
		//������ռ�����
		PwdIn_cnt = 0;
		PwdIn_Buf[PwdIn_cnt++]=KeyState;
		PwdIn_Buf[PwdIn_cnt]='\0';
		OLED_ShowString(8,2,(u8 *)PwdIn_Buf,16);
		// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
		KeyState = ' ';
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
				if(PwdIn_cnt < 16)
				{
					PwdIn_Buf[PwdIn_cnt++]=KeyState;
					PwdIn_Buf[PwdIn_cnt]='\0';
					OLED_ShowString(8,2,(u8 *)PwdIn_Buf,16);
				}
				// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
				KeyState = ' ';
			}
			else if(KeyState == '*')  //*��ʾȷ�ϼ�
			{
				OLED_Clear();
				if(strstr((char *)PwdIn_Buf,(char *)pwd_buf)!=NULL)
				{
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
					OLED_Clear();
					OLED_ShowCHinese(0,3,0);	       //С
					OLED_ShowCHinese(18,3,1);	       //��
					OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
					OLED_ShowCHinese(92,3,2);	       //��
					OLED_ShowCHinese(110,3,3);	     //��
				}
				else
				{
					//�������
					OLED_ShowCHinese(32,3,103);	       
					OLED_ShowCHinese(48,3,81);	       
					OLED_ShowCHinese(64,3,9);	      
					OLED_ShowCHinese(80,3,10);	 
					PwdAlarmLimit++;
					if(PwdAlarmLimit==8) //����������������8�ξͱ���
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
					for(int i=0;i<4;i++)
					{
						delay_ms(500); //���ܳ���798
					}
					OLED_Clear();
					OLED_ShowCHinese(0,3,0);	       //С
					OLED_ShowCHinese(18,3,1);	       //��
					OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
					OLED_ShowCHinese(92,3,2);	       //��
					OLED_ShowCHinese(110,3,3);	     //��
				}
				
				// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
				KeyState = ' ';
				break;
			}
			else if(KeyState == '#')  //#��ʾ�����
			{
				OLED_Clear();
				//��ս��ջ�����
				memset((char *)PwdIn_cnt,0,16);
				//������ռ�����
				PwdIn_cnt = 0;
				// ��������Ϻ󽫰���״̬����Ϊ' '�������ظ�����ͬһ�������¼�
				KeyState = ' ';
				OLED_ShowCHinese(0,3,0);	       //С
				OLED_ShowCHinese(18,3,1);	       //��
				OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
				OLED_ShowCHinese(92,3,2);	       //��
				OLED_ShowCHinese(110,3,3);	     //��
				
				ClearFlag=1;
			}	
		}
	}
}


//�������ź���
void ReadCardOpen(void)
{
	status=MFRC522_Request(0x52, (u8 *)card_pydebuf);			//Ѱ��   S50���Ŀ�����ȫ��Ψһ��  ���ڲ�����Ȧ  ���ö�ȡ����ȡ��Ƭ��Ϣ  ��Ƶ 13.56MHZ  
	
	if(status==0)		//���������
	{
		MFRC522_Anticoll((u8 *)card_numberbuf);			//����ײ����  �������ṩ�����Žϴ�Ŀ�Ƭ  ���Եõ�����  ����4�ֽ�
		char card_tmpid[20]="0x";
		for(int i=0;i<4;i++)
		{
			sprintf(card_tmpid+2+2*i,"%02X",card_numberbuf[i]);
		}
		for(int i=0;i<card_cnt;i++)
		{
			if(strstr((char *)card_tmpid,(char*)card_id[i])!=NULL) //�ȶ�
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
				OLED_ShowString(32,6,(u8 *)card_tmpid,16);
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
				PwdAlarmLimit=0; //��λ����ֹ���󱨾�
				OLED_Clear(); 	//����
				OLED_ShowCHinese(0,3,0);	       //С
				OLED_ShowCHinese(18,3,1);	       //��
				OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
				OLED_ShowCHinese(92,3,2);	       //��
				OLED_ShowCHinese(110,3,3);	     //��
				status=1;
				return; //�����������ȶ�
			}
		}
		OLED_Clear(); 	//����
		OLED_ShowCHinese(16,3,2);	    //�ſ�û��ע��   
		OLED_ShowCHinese(32,3,94);	  
		OLED_ShowCHinese(48,3,15);	     
		OLED_ShowCHinese(64,3,16);	       
		OLED_ShowCHinese(80,3,101);	       
		OLED_ShowCHinese(96,3,102);	      
		 
		delay_ms(500); //���ܳ���798
		OLED_Clear(); 	//����
		OLED_ShowCHinese(0,3,0);	       //С
		OLED_ShowCHinese(18,3,1);	       //��
		OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
		OLED_ShowCHinese(92,3,2);	       //��
		OLED_ShowCHinese(110,3,3);	     //��

		PwdAlarmLimit++;
		if(PwdAlarmLimit==8) //����������������5�ξͱ���
		{
			OLED_Clear(); 	//����
			OLED_ShowCHinese(16,3,2);	    //�ſ�û��ע��   
			OLED_ShowCHinese(32,3,94);	  
			OLED_ShowCHinese(48,3,15);	     
			OLED_ShowCHinese(64,3,16);	       
			OLED_ShowCHinese(80,3,101);	       
			OLED_ShowCHinese(96,3,102);	
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
			OLED_Clear(); 	//����
			OLED_ShowCHinese(0,3,0);	       //С
			OLED_ShowCHinese(18,3,1);	       //��
			OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾlogoͼƬ
			OLED_ShowCHinese(92,3,2);	       //��
			OLED_ShowCHinese(110,3,3);	     //��
			PwdAlarmLimit=0; //��λ����ֹ���󱨾�
		}
		//�ָ�����״̬
		status=1;
	}
}



TaskHandle_t task1_handle = NULL;  //����1�ľ��
TaskHandle_t task2_handle = NULL;  //����2�ľ��


//��������
void task1_func(void *arg)
{
	printf("task1 create success!\r\n");

	//��������ѭ��
	while(1)
	{
		//����WiFi��STA����ģʽ����
		while( Esp8266_Init()&&timeflag==0);							
		timeflag=0;//��ʱ���
		//���Ӱ����Ʒ�����
		while( Mqtt_Connect_Aliyun()&&timeflag==0);					//����MQTT���Ӱ�����	
		//TIM2�ĳ�ʼ��(MQTT����������)   
		TIM2_init();
		//5.�رն�ʱ��
		TIM_Cmd(TIM6, DISABLE);
		vTaskDelay( 500 );								//������ʱ
	}
}

//��������
void task2_func(void *arg)
{
	printf("task2 create success!\r\n");

	//��������ѭ��
	while(1)
	{	
		vTaskSuspendAll( );							//���������
//		printf("task2 is running!\r\n");
		
		//���������·�����
		mqtt_msg_handle();
		
		
		//��ȡ����
		get_key();
		//��ȡ�������벢�ж��Ƿ���ȷ
		PasswordInput();
		
		//ˢָ��
		press_FR();
		//�������ź���
		ReadCardOpen();
		
		//�ж������Ƿ���յ����ݲ����д���
		BlueRecvProcessing();
		
//		xTaskResumeAll();								//�ָ�������
	}
}





//�������
int main()
{
	USART1_Config(9600);
	
	//1.Ӳ����ʼ��
	BLE_Config(9600); //������ʼ��
	
	MFRC522_Initializtion(); //RFID�ĳ�ʼ��
	
	Relay_Config(); //�̵�����ʼ��
	
	Beep_Config();  //��������ʼ��
	
	Key_Init(); //���̳�ʼ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //�ж����ȼ����� 4bit����ռ���ȼ���0~15��,�����ȼ��ǹ̶�Ϊ0
	//USART2�ĳ�ʼ��
	USART2_Config(57600);
	
	OLED_Init();		//��ʼ��OLED 
		
	OLED_Clear();  //����
	
	UART4_Config(115200);
	
	//TIM6�ĳ�ʼ��(��ʱ���)
	TIM6_Config();
	
	OLED_ShowCHinese(0,3,0);	       //С
	OLED_ShowCHinese(18,3,1);	       //��
	OLED_DrawBMP(46,1,81,7,BMP1);    //��ʾͼƬ
	OLED_ShowCHinese(92,3,2);	       //��
	OLED_ShowCHinese(110,3,3);	     //��
	
	//1.��������
	xTaskCreate(    task1_func,												//�������ָ��
															"task1",													//��������
															3000,															//��ջ��С 128 * 32 / 8 = 512�ֽ�
															NULL,														  //����Ҫ�����񴫲�
															2,																//�������ȼ�  ����Խ�������ȼ�Խ�ߣ����Ҳ�Ҫ�������ȼ����ֵ
															&task1_handle											//����ľ���������ṩ������API�ӿ�ʹ��
												 );
	
	xTaskCreate(    task2_func,												//�������ָ��
															"task2",													//��������
															3000,															//��ջ��С 128 * 32 / 8 = 512�ֽ�
															NULL,														  //����Ҫ�����񴫲�
															1,																//�������ȼ�  ����Խ�������ȼ�Խ�ߣ����Ҳ�Ҫ�������ȼ����ֵ
															&task2_handle											//����ľ���������ṩ������API�ӿ�ʹ��
												 );
	
	//2.��������������仰����Ĵ��붼����Ч��
	vTaskStartScheduler();
 
}


