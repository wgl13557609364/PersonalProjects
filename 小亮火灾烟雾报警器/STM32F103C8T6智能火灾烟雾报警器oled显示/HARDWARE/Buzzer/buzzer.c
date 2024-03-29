/************************************************************************************

//  ��������   : ������ģ����Գ���
//  ��������   : STM32F103C8T6   ����8M  ϵͳʱ�� 72M

����

VCC-- --------------------------------3.3V
GND- ---------------------------------GND
B15- ----------------------------------PB_15 

*****************************************************************************************/


#include "buzzer.h"
#include "stm32f10x.h"

//����������������Ϊ���ģʽ B������
void  BUZZER_GPIO_Config(uint16_t GPIO_Pin_x)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//Ĭ������ߵ�ƽ
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
}
