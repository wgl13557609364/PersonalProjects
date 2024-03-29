#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f4xx.h"

void TIM7_Int_Init(u16 arr,u16 psc);
//TIM6的初始化(MQTT发送数据)
void TIM6_Config(void);
//TIM2的初始化(MQTT发送心跳包)   
void TIM2_init(void);
 
#endif
