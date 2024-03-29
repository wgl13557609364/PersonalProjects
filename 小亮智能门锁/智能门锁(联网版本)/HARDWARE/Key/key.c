/************************************************************************************
//  �� �� ��   : 
//  ��    ��   : 
//  ��������   : 
//  ����޸�   : 
//  ��������   : ���󰴼�ģ����Գ���
//  ��������   : STM32F407ZG   ����8M  ϵͳʱ�� 168M
����

���󰴼�ģ��------------------------------------STM32F407ZG 
//����˿ӡ ���ϵ�������ΪK8-K1
K8-----------------------------------------------PC8 ������
K7-----------------------------------------------PC7 ������
K6-----------------------------------------------PC6 �ڶ���
K5-----------------------------------------------PC5 ��һ��
K4-----------------------------------------------PC4 ������
K3-----------------------------------------------PC3 ������
K2-----------------------------------------------PC9 �ڶ���
K1-----------------------------------------------PC1 ��һ��

OLED0.96
VCC-- -------------------------------------------3.3V
GND- --------------------------------------------GND
SCL- --------------------------------------------PE7 //SCL
SDA- --------------------------------------------PE8 //SDA	
*****************************************************************************************/

#include "key.h"
#include "oled.h"
#include "delay.h"


char KeyState = ' ';

__IO u8 keyflag1,keyflag2,keyflag3,keyflag4,keyflag5,keyflag6,keyflag7,keyflag8,
keyflag9,keyflag10,keyflag11,keyflag12,keyflag13,keyflag14,keyflag15,keyflag16; // ȫ�ֱ���,Ĭ�ϳ�ʼֵΪ0���жϰ����Ƿ�ı䰴��״̬

void Key_Init(void)
{
	//����ṹ��
	GPIO_InitTypeDef GPIO_InitStruct;

	//��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//���úͳ�ʼ������ ������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_9 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; //����ʹ����������GPIO_PuPd_UP ʹ��ƽĬ��Ϊ�ߵ�ƽ1 
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	//���úͳ�ʼ������ ������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; //��ʹ������������GPIO_PuPd_NOPULL
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void get_key(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_5);  // ����1���õ�  Ŀ����ʹ������·
	GPIO_SetBits(GPIOC, GPIO_Pin_6);    // ����2���ø�
	GPIO_SetBits(GPIOC, GPIO_Pin_7);    // ����3���ø�
	GPIO_SetBits(GPIOC, GPIO_Pin_8);    // ����4���ø�

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag1 == 0) //��ⰴ��1
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '1';
		keyflag1=1;  //������������ֵ,ֱ���ɿ�����
	}
	else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag1=0; //��������ֵ
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag4 == 0) //��ⰴ��4
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '4';
		keyflag4 = 1; //������������ֵ,ֱ���ɿ�����
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		keyflag4 = 0; //��������ֵ
		delay_ms(5); //�ɿ�����ʱ����
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag7 == 0) //��ⰴ��7
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '7';
		keyflag7 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag7 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag11 == 0) //��ⰴ��*
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '*';
		keyflag11 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag11 = 0;
	}

	GPIO_SetBits(GPIOC, GPIO_Pin_5);   // ����1���ø�
	GPIO_ResetBits(GPIOC, GPIO_Pin_6); // ����2���õ�  Ŀ����ʹ������·
	GPIO_SetBits(GPIOC, GPIO_Pin_7);   // ����3���ø�
	GPIO_SetBits(GPIOC, GPIO_Pin_8);   // ����4���ø�

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag2 == 0) //��ⰴ��2
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '2';
		keyflag2 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag2 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag5 == 0) //��ⰴ��5
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '5';
		keyflag5 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag5 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag8 == 0) //��ⰴ��8
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '8';
		keyflag8 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag8 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag10 == 0) //��ⰴ��0
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '0';
		keyflag10 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag10 = 0;
	}

	GPIO_SetBits(GPIOC, GPIO_Pin_5);    // ����1���ø�
	GPIO_SetBits(GPIOC, GPIO_Pin_6);    // ����2���ø�
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);  // ����3���õ�   Ŀ����ʹ������·
	GPIO_SetBits(GPIOC, GPIO_Pin_8);    // ����4���ø�

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag3 == 0) //��ⰴ��3
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '3';
		keyflag3 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag3 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag6 == 0) //��ⰴ��6
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '6';
		keyflag6 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag6 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag9 == 0) //��ⰴ��9
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '9';
		keyflag9 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag9 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag12 == 0) //��ⰴ��#
	{
		delay_ms(5); //���°���ʱ����
		KeyState = '#';
		keyflag12 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag12 = 0;
	}

	GPIO_SetBits(GPIOC, GPIO_Pin_5);      // ����1���ø�
	GPIO_SetBits(GPIOC, GPIO_Pin_6);      // ����2���ø�
	GPIO_SetBits(GPIOC, GPIO_Pin_7);      // ����3���ø�   
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);    // ����4���õ�   Ŀ����ʹ������·

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag13 == 0) //��ⰴ��A
	{
		delay_ms(5); //���°���ʱ����
		KeyState = 'A';
		keyflag13 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag13 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag14 == 0) //��ⰴ��B
	{
		delay_ms(5); //���°���ʱ����
		KeyState = 'B';
		keyflag14 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag14 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag15 == 0) //��ⰴ��C
	{
		delay_ms(5); //���°���ʱ����
		KeyState = 'C';
		keyflag15 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag15 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag16 == 0) //��ⰴ��D
	{
		delay_ms(5); //���°���ʱ����
		KeyState = 'D';
		keyflag16 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //�ɿ�����ʱ����
		keyflag16 = 0;
	}

//	// ִ����Ӧ�Ĳ���
//	if (KeyState != ' ') 
//	{
//		// ���µļ���Ч ������Ҫ������ִ����Ӧ�Ĳ����������ӡ���µļ�ֵ
//		LED_ShowStringar(8,2 ,KeyState,16);
//	//			delay_ms(500); //���ܳ���798
//	//			delay_ms(500); //���ܳ���798
//	}

//	// ������״̬����Ϊ' '�������ظ�����ͬһ�������¼�
//	KeyState = ' ';

}









