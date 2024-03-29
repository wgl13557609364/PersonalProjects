/************************************************************************************



*************************************************************************************/

#include "stm32f10x.h"
#include "delay.h"

static u8  fac_us=0;//us
static u16 fac_ms=0;//ms


void DelayInit()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	
	fac_us=SystemCoreClock/8000000;	
	fac_ms=(u16)fac_us*1000;
}

	    								   
void Delay_us(unsigned long nus)//微秒级别
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 		 
	SysTick->VAL=0x00;       
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;      
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       
	SysTick->VAL =0X00;        
}

void Delay_ms(unsigned int nms) //毫秒级别
{
	u32 temp;
	SysTick->LOAD=(u32)nms*fac_ms;
	SysTick->VAL =0x00;           
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;         
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       
	SysTick->VAL =0X00;      
}

void Delay_s(unsigned int ns)//秒级别
{
	unsigned char i;
	for(i=0;i<ns;i++)
	{
		Delay_ms(1000);
	}
}
