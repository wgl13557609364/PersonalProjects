#include "timer.h"
#include "key.h" 	 
#include "usart.h"
#include "mqtt.h"
extern vu16 USART2_RX_STA;
u8 key_num=0;
//通用定时器7中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M 则时钟为84M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.

volatile int timeflag=0;

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



//TIM6的初始化(MQTT发送数据)
void TIM6_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//1.打开TIM6的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
	//2.配置定时时间  假设2000ms	
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;	//TIM6的频率是84MHZ  84000000HZ/8400 = 1000HZ -->计数1次周期为100us
	TIM_TimeBaseStructure.TIM_Period 		= 20000-1;	//打算定时2000ms  2000ms * 1000 / 100 = 20000
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数，基本定时器只支持递增计数
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	//3.配置NVIC外设并初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;							//中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;				//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;							//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							    //中断通道使能
	NVIC_Init(&NVIC_InitStructure);
	
	//4.选择定时器的中断源
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	
	//5.打开定时器
	TIM_Cmd(TIM6, ENABLE);
	
}
int j=0;

//利用MQTT上传给阿里云 每隔2s触发一次
void TIM6_DAC_IRQHandler(void)
{
	//判断中断是否触发
	if( TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET )
	{
		j++;
		if(j>40)
		{
			timeflag=1;
		}
		//清除中断标志
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}

}



int i = 0;
//TIM2的初始化(MQTT发送心跳包)   
void TIM2_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1; // 84MHz时钟，分频为8400，计数频率为10000Hz
    TIM_TimeBaseStructure.TIM_Period = 30000 - 1; // 计数器重装载值，定时3秒
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM2, ENABLE);


}



//利用定时器定时向服务器发送响应，避免掉线
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
			i++;
			if(i>10)
			{
				i=0;
				mqtt_send_heart(); //向服务器发送心跳包
			}
			//清除中断标志
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}














