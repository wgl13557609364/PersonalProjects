#include "led.h"

#include "stm32f10x.h"

//LED����������Ϊ���ģʽ B������
void  LED_GPIO_Config(uint16_t GPIO_Pin_x)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//Ĭ������ߵ�ƽ
	GPIO_SetBits(GPIOC, GPIO_Pin_x);
}
