#include "timer.h"
#include "key.h" 	 
#include "usart.h"

extern vu16 USART2_RX_STA;
u8 key_num=0;
//通用定时器7中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M 则时钟为84M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
	 
void TIM7_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//1.打开TIM7的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);   
	
	//2.配置TIM7定时时间  	
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_Period 		= arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数，基本定时器只支持递增计数
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	//3.配置NVIC外设并初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;							//中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;				//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;							//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							    //中断通道使能
	NVIC_Init(&NVIC_InitStructure);
	
	//4.选择定时器的中断源
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
	
	//5.打开定时器
	TIM_Cmd(TIM7, ENABLE);
}


//定时器7中断服务程序		    
void TIM7_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		USART2_RX_STA|=1<<15;	//标记接收完成
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //清除TIM7更新中断标志    
		TIM_Cmd(TIM7, DISABLE);  //关闭TIM7 
	}	    
}














