#ifndef __DS18B20_H_
#define __DS18B20_H_

/**********************************
包含头文件
**********************************/
#include "stm32f10x.h"

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint 
#define uint unsigned int
#endif

#define  DS18B20_DAT_1   		GPIO_SetBits(GPIOA,GPIO_Pin_10)
#define  DS18B20_DAT_0   		GPIO_ResetBits(GPIOA,GPIO_Pin_10)
#define  DS18B20_DAT_Read   GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)

/**********************************
函数声明
**********************************/

extern unsigned char  DisplayData[15];
extern int D_temp;

void DS18B20_OUT_GPIO_Config(void);
int Ds18b20ReadTemp(void);
void datapros(int temp);
#endif





