#include "delay.h"

//��ʱ΢��  ��������ʱ
void delay_us(u32 nus)
{
	int cnt  = 0;  //���ڴ洢�������ܴ���
	int load = 0; //���ڼ�¼Systick���Զ����ؼĴ�����ֵ
	int told = 0; //���ڼ�¼Systick�ĵ�ǰ��ֵ�Ĵ����ĳ�ֵ
	int tnew = 0; //���ڼ�¼Systick�ĵ�ǰ��ֵ�Ĵ�������ֵ
	int sum  = 0; //��¼Systick�ļ�������
	
	//1.������ʱʱ���Ӧ�ļ�������  Systick��ʱ��Դ��168MHZ ����1us����168�� 
	cnt = nus * 168;
	
	//2.��¼Systick���Զ����ؼĴ�����ֵ
	load = SysTick->LOAD;
	
	//3.��¼Systick�ĵ�ǰ��ֵ�Ĵ����ĳ�ֵ
	told = SysTick->VAL;
	
	//4.ѭ����¼��ǰ��ֵ�Ĵ����ļ�����������Ҫ��ʱ�ļ����������бȽϼ���
	while(1)
	{
		//5.��ȡSystick�ĵ�ǰ��ֵ�Ĵ�����ֵ
		tnew = SysTick->VAL;
		
		//6.�ж��Ƿ����һ������
		if(told != tnew)
		{
			if(told < tnew)
					sum += load - tnew + told;
			else
					sum += told - tnew;
			
			told = tnew;
			
			if(sum >= cnt) //˵��ʱ�䵽��
				break;
		}
	}
}

//��ʱ����  ��������ʱ
void delay_ms(u32 nms)
{
	int i = 0;
	for(i=0;i<nms;i++)
		delay_us(1000);
}
































