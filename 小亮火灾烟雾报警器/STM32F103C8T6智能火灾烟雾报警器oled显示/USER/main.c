/************************************************************************************

//  ��������   : STM32F103C8T6   ����8M  ϵͳʱ�� 72M

����
MQ2-----------------------------------STM32F103C8T6
VCC-- --------------------------------5V
GND- ---------------------------------GND
A0- ----------------------------------PA_0 

OLED0.96
VCC-- --------------------------------5V
GND- ---------------------------------GND
SCL- ---------------------------------PB_8 //SCL
SDA- ---------------------------------PB_9 //SDA	
*****************************************************************************************/


#include "stm32f10x.h"
#include "bsp_adc.h"
#include "delay.h"
#include "oled.h"
#include "buzzer.h"
#include "led.h"
#include "ds18b20.h"
#include<stdlib.h>
#include<stdio.h> 

#define KEY1_GPIO GPIO_Pin_9
#define KEY2_GPIO GPIO_Pin_8
#define KEY3_GPIO GPIO_Pin_7
#define KEY4_GPIO GPIO_Pin_6


extern __IO uint16_t ADC_ConvertedValue; //ģ��ת��ֵ

__IO float Vol;  // ȫ�ֱ��������ڱ���ת�������ĵ�ѹֵ 	
__IO float smoke;  // ȫ�ֱ��������ڱ���ת������������Ũ��
__IO u8 keyState = 0; // ȫ�ֱ���������״̬������������ĸ�����������
__IO u8 key1flag,key2flag,key3flag,key4flag; // ȫ�ֱ���,Ĭ�ϳ�ʼֵΪ0���жϰ����Ƿ�ı䰴��״̬
__IO float temp; //�洢�¶�ֵ
__IO int Handover =0; //����������־
__IO int show_flag = 0; //ȫ�ֱ����������л���ҳ�������ҳ��
__IO int set_temp=45; //��ʼ�¶ȱ���ֵ 45
__IO int set_smoke=40; //��ʼ������ֵ 40
__IO int ManualAlarm = 0; //�ֶ�������־λ


//char Dis_Vol_Buf[20]={0};  //��ѹ
char Dis_Smoke_Buf[20]={0};  //����
char Dis_ADC_Buf[20]={0};  //������
char Dis_Temp_Buf[20]={0}; //�¶�


//****************************************
//����ת�ַ���
//****************************************
void OLED_printf(unsigned char *s,int temp_data)
{
	if(temp_data<0)
	{
		temp_data=-temp_data;
		*s='-';
	}
	else *s=' ';

//	*++s =temp_data/10000+0x30;
//	temp_data=temp_data%10000;     //ȡ������
	*++s ='T';
	*++s ='e';
	*++s ='m';
	*++s ='p';
	*++s =':';	
	*++s =temp_data/1000+0x30;
	temp_data=temp_data%1000;     //ȡ������

	*++s =temp_data/100+0x30;
	*++s ='.';
	temp_data=temp_data%100;     //ȡ������
	*++s =temp_data/10+0x30;
	temp_data=temp_data%10;      //ȡ������
	*++s =temp_data+0x30; 
	*++s ='C';	
	
	*++s ='\0'; 

}

//GPIO_Mode_AIN��ģ������ģʽ��
//GPIO_Mode_IN_FLOATING����������ģʽ�����ڲ����������衣
//GPIO_Mode_IPD����������ģʽ���ڲ��������豻���á�
//GPIO_Mode_IPU����������ģʽ���ڲ��������豻���á�
//GPIO_Mode_Out_OD����©���ģʽ���������ⲿ����������·��
//GPIO_Mode_Out_PP���������ģʽ�������ṩ�ϸߵ����������
//GPIO_Mode_AF_OD����©����ģʽ���������ⲿ����������·�������ض��Ĺ��ܸ��á�
//GPIO_Mode_AF_PP�����츴��ģʽ�������ض��Ĺ��ܸ��ã������ṩ�ϸߵ����������


//****************************************
//������ʼ������
//****************************************
void  Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

	//KEY1 //KEY2
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO | KEY2_GPIO | KEY3_GPIO | KEY4_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}


//****************************************
//������⺯��
//****************************************
void KeyDetection(void)  
{
	if (GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//������������	
		if (GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO) == Bit_RESET && key1flag==0)
		{
			keyState=1;
			key1flag=1; //���ţ����ð���״̬��Ϊ��ƽ������θ�ֵ
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO) == Bit_SET)
	{
		Delay_ms(5);//������������	
		key1flag=0; //���ţ���������ֵ
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//������������	
		if (GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO) == Bit_RESET && key2flag==0)
		{
			keyState=2;
			key2flag=1; //���ţ����ð���״̬��Ϊ��ƽ������θ�ֵ
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO) == Bit_SET)
	{
		Delay_ms(5);//������������	
		key2flag=0; //���ţ���������ֵ
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//������������	
		if (GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO) == Bit_RESET && key3flag==0)
		{
			keyState=3;
			key3flag=1; //���ţ����ð���״̬��Ϊ��ƽ������θ�ֵ
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO) == Bit_SET)
	{
		Delay_ms(5);//������������	
		key3flag=0; //���ţ���������ֵ
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//������������	
		if (GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO) == Bit_RESET && key4flag==0)
		{
			keyState=4;
			key4flag=1; //���ţ����ð���״̬��Ϊ��ƽ������θ�ֵ
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO) == Bit_SET)
	{
		Delay_ms(5);//������������	
		key4flag=0; //���ţ���������ֵ
	}
	
}


//****************************************
//��ҳ�溯��
//****************************************
void show_main(void)
{
	Vol =(float) 3.3*ADC_ConvertedValue/4096; 
	smoke = (Vol-0.27)/3.03*100;
	datapros(Ds18b20ReadTemp());
	sprintf(Dis_Smoke_Buf," Smoke:%.2f%% ",smoke);
	//		sprintf(Dis_Vol_Buf," Vol:%.2f ",Vol);
	//		sprintf(Dis_ADC_Buf,"AD_VA:%d   ",ADC_ConvertedValue);
	OLED_printf(DisplayData,D_temp);
	if(smoke>set_smoke)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_ResetBits(GPIOC, GPIO_Pin_15);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_15);
	}


	sscanf((const char *)DisplayData," Temp:%fC",&temp);
	if(temp > set_temp)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}

	//�رշ�����
	if(smoke<=set_smoke && temp<=set_temp)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_15);
	}

	OLED_ShowString(0,2,(u8 *)Dis_Smoke_Buf,16);
	//		OLED_ShowString(0,0,(u8 *)Dis_ADC_Buf,16);
	OLED_ShowString(0,4,(u8 *)DisplayData,16); 
	keyState = 0; //�������״̬
}


//****************************************
//����ҳ�溯��
//****************************************
void show_set(void)
{
	KeyDetection(); // ��ȡ����״̬
	
	//������������
	if(show_flag==1)
	{
		if(Handover >= 1)
		{
			OLED_Clear();
			Handover = 0;
		}
		
		if(keyState==1) //�Ӽ�
		{
			set_smoke++;
			if(set_smoke>=99)
			set_smoke=99;
		}
		
		if(keyState==2) //����
		{
			set_smoke--;
			if(set_smoke==9)
			{
				OLED_Clear();
			}
			if(set_smoke<=0)
			set_smoke=0;
		}
		OLED_ShowString(8,3,(u8 *)"SmokeAlarm:",16);
		OLED_ShowNumber(96,3,set_smoke,2,16);
		keyState = 0; //�������״̬
		
	}
	
	
	//�����¶ȱ�����
	if(show_flag==2)
	{
		if(Handover >= 1)
		{
			OLED_Clear();
			Handover = 0;
		}
		
		if(keyState==1)
		{
			set_temp++;
			if(set_temp>=99)
			set_temp=99;
		}
		
		if(keyState==2)
		{
			set_temp--;
			if(set_temp==9)
			{
				OLED_Clear();
			}
			if(set_temp<=0)
			set_temp=0;
		}
		 
		OLED_ShowString(8,2,(u8 *)"TempAlarm:",16);
		OLED_ShowNumber(88,2,set_temp,2,16);
		keyState = 0; //�������״̬
	}	
}

//****************************************
//��������
//****************************************
void AlarmFunction(int ManualAlarm)
{
	//�ж��Ƿ�����ֶ�����flag
		if(ManualAlarm == 1 )
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_15);  //�͵�ƽ��
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);  //�͵�ƽ��
			GPIO_ResetBits(GPIOC, GPIO_Pin_15);  //�͵�ƽ��
			
		}
		else
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_15);  
			GPIO_SetBits(GPIOC, GPIO_Pin_13);  
			GPIO_SetBits(GPIOC, GPIO_Pin_15);  
			
		}
}








/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{	
	
	// ��ʼ��
	BUZZER_GPIO_Config(GPIO_Pin_15); //B������
	LED_GPIO_Config(GPIO_Pin_13); //C������
	LED_GPIO_Config(GPIO_Pin_15); //C������
	ADCx_Init(); //ģʽת����ʼ��
	DelayInit(); //��ʼ���ӳٺ���
	OLED_Init(); //��ʼ��OLED��Ļ
	OLED_Clear(); //����
	DS18B20_OUT_GPIO_Config(); //��ʼ���¶ȴ���������
	Key_GPIO_Config(); //��ʼ������
	while (1)
	{
		KeyDetection(); // ��ȡ����״̬
		
		if(keyState==3)//������
		{
			if(ManualAlarm == 0)
			{
				//�ֶ���������־λ 
				ManualAlarm = 1;
			}
			else
			{
				//�ֶ���������־λ 
				ManualAlarm = 0;
			}
			keyState = 0; //�������״̬
		}
		
		if(keyState==4)//���ü�
		{
			if(ManualAlarm != 1)
			{
				show_flag++; //��ҳ�������ҳ���л���־λ
				if(show_flag>=3)
				show_flag=0;
				Handover++; //�л�ҳ���־λ
			}
		}
		
		//����show_flag = 0����ʾ��ҳ�棬ManualAlarm = 1�򴥷�������Handover >= 1���ʾ�л���ҳ�� 
		if(show_flag == 0&&ManualAlarm != 1) //ManualAlarm=1ʱ�����Եõ��ֶ�����ֵ�Ĺ̶�ֵ������ֵ���ٷ����仯
		{
			
			if(Handover >= 1)
			{
				OLED_Clear();
				Handover = 0;
			}
			
			show_main(); //��ҳ��
			

		}
		
		//����show_flag == 1||show_flag == 2����ʾ������ֵ����ҳ�棬1��ʾ����Ũ������ҳ���2��ʾ�¶�����ҳ��
		if(show_flag == 1||show_flag == 2)
		{
			if(Handover >= 1)
			{
				OLED_Clear();
				Handover = 0;
			}
			show_set(); //����ҳ��
			
		}
		
		//��������
		AlarmFunction(ManualAlarm);
        
	}
	
}






