#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f4xx.h"

void TIM7_Int_Init(u16 arr,u16 psc);
//TIM6�ĳ�ʼ��(MQTT��������)
void TIM6_Config(void);
//TIM2�ĳ�ʼ��(MQTT����������)   
void TIM2_init(void);
 
#endif
