/************************************************************************************

//  功能描述   : 蜂鸣器模块测试程序
//  测试条件   : STM32F103C8T6   晶振8M  系统时钟 72M

接线

VCC-- --------------------------------3.3V
GND- ---------------------------------GND
B15- ----------------------------------PB_15 

*****************************************************************************************/


#include "buzzer.h"
#include "stm32f10x.h"

//蜂鸣器的引脚配置为输出模式 B组引脚
void  BUZZER_GPIO_Config(uint16_t GPIO_Pin_x)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//默认输出高电平
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
}
