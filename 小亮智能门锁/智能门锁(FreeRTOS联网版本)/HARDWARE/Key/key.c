/************************************************************************************
//  版 本 号   : 
//  作    者   : 
//  生成日期   : 
//  最近修改   : 
//  功能描述   : 矩阵按键模块测试程序
//  测试条件   : STM32F407ZG   晶振8M  系统时钟 168M
接线

矩阵按键模块------------------------------------STM32F407ZG 
//正对丝印 从上到下引脚为K8-K1
K8-----------------------------------------------PC8 第四列
K7-----------------------------------------------PC7 第三列
K6-----------------------------------------------PC6 第二列
K5-----------------------------------------------PC5 第一列
K4-----------------------------------------------PC4 第四行
K3-----------------------------------------------PC3 第三行
K2-----------------------------------------------PC9 第二行
K1-----------------------------------------------PC1 第一行

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
keyflag9,keyflag10,keyflag11,keyflag12,keyflag13,keyflag14,keyflag15,keyflag16; // 全局变量,默认初始值为0，判断按键是否改变按键状态

void Key_Init(void)
{
	//定义结构体
	GPIO_InitTypeDef GPIO_InitStruct;

	//打开时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//配置和初始化引脚 行引脚
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_9 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; //必须使用上拉电阻GPIO_PuPd_UP 使电平默认为高电平1 
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	//配置和初始化引脚 列引脚
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; //不使用上下拉电阻GPIO_PuPd_NOPULL
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void get_key(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_5);  // 将第1列置低  目的是使按键短路
	GPIO_SetBits(GPIOC, GPIO_Pin_6);    // 将第2列置高
	GPIO_SetBits(GPIOC, GPIO_Pin_7);    // 将第3列置高
	GPIO_SetBits(GPIOC, GPIO_Pin_8);    // 将第4列置高

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag1 == 0) //检测按键1
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '1';
		keyflag1=1;  //不再允许按键赋值,直到松开按键
	}
	else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag1=0; //允许按键赋值
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag4 == 0) //检测按键4
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '4';
		keyflag4 = 1; //不再允许按键赋值,直到松开按键
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		keyflag4 = 0; //允许按键赋值
		delay_ms(5); //松开按键时消抖
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag7 == 0) //检测按键7
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '7';
		keyflag7 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag7 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag11 == 0) //检测按键*
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '*';
		keyflag11 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag11 = 0;
	}

	GPIO_SetBits(GPIOC, GPIO_Pin_5);   // 将第1列置高
	GPIO_ResetBits(GPIOC, GPIO_Pin_6); // 将第2列置低  目的是使按键短路
	GPIO_SetBits(GPIOC, GPIO_Pin_7);   // 将第3列置高
	GPIO_SetBits(GPIOC, GPIO_Pin_8);   // 将第4列置高

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag2 == 0) //检测按键2
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '2';
		keyflag2 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag2 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag5 == 0) //检测按键5
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '5';
		keyflag5 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag5 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag8 == 0) //检测按键8
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '8';
		keyflag8 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag8 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag10 == 0) //检测按键0
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '0';
		keyflag10 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag10 = 0;
	}

	GPIO_SetBits(GPIOC, GPIO_Pin_5);    // 将第1列置高
	GPIO_SetBits(GPIOC, GPIO_Pin_6);    // 将第2列置高
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);  // 将第3列置低   目的是使按键短路
	GPIO_SetBits(GPIOC, GPIO_Pin_8);    // 将第4列置高

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag3 == 0) //检测按键3
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '3';
		keyflag3 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag3 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag6 == 0) //检测按键6
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '6';
		keyflag6 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag6 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag9 == 0) //检测按键9
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '9';
		keyflag9 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag9 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag12 == 0) //检测按键#
	{
		delay_ms(5); //按下按键时消抖
		KeyState = '#';
		keyflag12 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag12 = 0;
	}

	GPIO_SetBits(GPIOC, GPIO_Pin_5);      // 将第1列置高
	GPIO_SetBits(GPIOC, GPIO_Pin_6);      // 将第2列置高
	GPIO_SetBits(GPIOC, GPIO_Pin_7);      // 将第3列置高   
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);    // 将第4列置低   目的是使按键短路

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 && keyflag13 == 0) //检测按键A
	{
		delay_ms(5); //按下按键时消抖
		KeyState = 'A';
		keyflag13 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag13 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0 && keyflag14 == 0) //检测按键B
	{
		delay_ms(5); //按下按键时消抖
		KeyState = 'B';
		keyflag14 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag14 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0 && keyflag15 == 0) //检测按键C
	{
		delay_ms(5); //按下按键时消抖
		KeyState = 'C';
		keyflag15 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag15 = 0;
	}
	
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 0 && keyflag16 == 0) //检测按键D
	{
		delay_ms(5); //按下按键时消抖
		KeyState = 'D';
		keyflag16 = 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1)
	{
		delay_ms(5); //松开按键时消抖
		keyflag16 = 0;
	}

//	// 执行相应的操作
//	if (KeyState != ' ') 
//	{
//		// 按下的键有效 根据需要在这里执行相应的操作，例如打印按下的键值
//		LED_ShowStringar(8,2 ,KeyState,16);
//	//			delay_ms(500); //不能超过798
//	//			delay_ms(500); //不能超过798
//	}

//	// 将按键状态重置为' '，避免重复处理同一个按键事件
//	KeyState = ' ';

}









