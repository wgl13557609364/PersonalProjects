#include "timer.h"
#include "key.h" 	 
#include "usart.h"
#include "mqtt.h"
extern vu16 USART2_RX_STA;
u8 key_num=0;
//ͨ�ö�ʱ��7�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ42M ��ʱ��Ϊ84M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.

volatile int timeflag=0;

void TIM7_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//1.��TIM7��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);   
	
	//2.����TIM7��ʱʱ��  	
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_Period 		= arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ�����������ʱ��ֻ֧�ֵ�������
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	//3.����NVIC���貢��ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;							//�ж�ͨ�����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;				//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;							//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							    //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	//4.ѡ��ʱ�����ж�Դ
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
	
	//5.�򿪶�ʱ��
	TIM_Cmd(TIM7, ENABLE);
}


//��ʱ��7�жϷ������		    
void TIM7_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		USART2_RX_STA|=1<<15;	//��ǽ������
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //���TIM7�����жϱ�־    
		TIM_Cmd(TIM7, DISABLE);  //�ر�TIM7 
	}	    
}



//TIM6�ĳ�ʼ��(MQTT��������)
void TIM6_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//1.��TIM6��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
	//2.���ö�ʱʱ��  ����2000ms	
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;	//TIM6��Ƶ����84MHZ  84000000HZ/8400 = 1000HZ -->����1������Ϊ100us
	TIM_TimeBaseStructure.TIM_Period 		= 20000-1;	//���㶨ʱ2000ms  2000ms * 1000 / 100 = 20000
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ�����������ʱ��ֻ֧�ֵ�������
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	//3.����NVIC���貢��ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;							//�ж�ͨ�����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;				//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;							//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							    //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	//4.ѡ��ʱ�����ж�Դ
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	
	//5.�򿪶�ʱ��
	TIM_Cmd(TIM6, ENABLE);
	
}
int j=0;

//����MQTT�ϴ��������� ÿ��2s����һ��
void TIM6_DAC_IRQHandler(void)
{
	//�ж��ж��Ƿ񴥷�
	if( TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET )
	{
		j++;
		if(j>40)
		{
			timeflag=1;
		}
		//����жϱ�־
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}

}



int i = 0;
//TIM2�ĳ�ʼ��(MQTT����������)   
void TIM2_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1; // 84MHzʱ�ӣ���ƵΪ8400������Ƶ��Ϊ10000Hz
    TIM_TimeBaseStructure.TIM_Period = 30000 - 1; // ��������װ��ֵ����ʱ3��
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



//���ö�ʱ����ʱ�������������Ӧ���������
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
			i++;
			if(i>10)
			{
				i=0;
				mqtt_send_heart(); //�����������������
			}
			//����жϱ�־
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}














