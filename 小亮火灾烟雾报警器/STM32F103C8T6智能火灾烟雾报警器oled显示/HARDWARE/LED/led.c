#include "led.h"

#include "stm32f10x.h"

//LED的引脚配置为输出模式 B组引脚
void  LED_GPIO_Config(uint16_t GPIO_Pin_x)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//默认输出高电平
	GPIO_SetBits(GPIOC, GPIO_Pin_x);
}
