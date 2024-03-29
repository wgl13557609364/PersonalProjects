#include "Relay.h"
#include "stm32f4xx.h"

//继电器初始化
void Relay_Config(void)
{
	/* Private typedef -----------------------------------------------------------*/
	GPIO_InitTypeDef  GPIO_InitStructure;  //定义外设结构体

  /* GPIOG Peripheral clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); //打开外设时钟
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  //给引脚赋值
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  //推挽模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //引脚输出速度
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //不使用上拉下拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);  //初始化
	
//	GPIO_SetBits(GPIOF, GPIO_Pin_15); //默认低电平触发
}
