/************************************************************************************

//  测试条件   : STM32F103C8T6   晶振8M  系统时钟 72M

接线
MQ2-----------------------------------STM32F103C8T6
VCC-- --------------------------------5V
GND- ---------------------------------GND
A0- ----------------------------------PA_0 

OLED0.96
VCC-- --------------------------------5V
GND- ---------------------------------GND
SCL- ---------------------------------PB_8 //SCL
SDA- ---------------------------------PB_9 //SDA	
*****************************************************************************************/


#include "stm32f10x.h"
#include "bsp_adc.h"
#include "delay.h"
#include "oled.h"
#include "buzzer.h"
#include "led.h"
#include "ds18b20.h"
#include<stdlib.h>
#include<stdio.h> 

#define KEY1_GPIO GPIO_Pin_9
#define KEY2_GPIO GPIO_Pin_8
#define KEY3_GPIO GPIO_Pin_7
#define KEY4_GPIO GPIO_Pin_6


extern __IO uint16_t ADC_ConvertedValue; //模数转换值

__IO float Vol;  // 全局变量，用于保存转换计算后的电压值 	
__IO float smoke;  // 全局变量，用于保存转换计算后的烟雾浓度
__IO u8 keyState = 0; // 全局变量，按键状态，用来检测是哪个按键按下了
__IO u8 key1flag,key2flag,key3flag,key4flag; // 全局变量,默认初始值为0，判断按键是否改变按键状态
__IO float temp; //存储温度值
__IO int Handover =0; //切屏清屏标志
__IO int show_flag = 0; //全局变量，用来切换主页面和设置页面
__IO int set_temp=45; //初始温度报警值 45
__IO int set_smoke=40; //初始烟雾报警值 40
__IO int ManualAlarm = 0; //手动报警标志位


//char Dis_Vol_Buf[20]={0};  //电压
char Dis_Smoke_Buf[20]={0};  //烟雾
char Dis_ADC_Buf[20]={0};  //数字量
char Dis_Temp_Buf[20]={0}; //温度


//****************************************
//整数转字符串
//****************************************
void OLED_printf(unsigned char *s,int temp_data)
{
	if(temp_data<0)
	{
		temp_data=-temp_data;
		*s='-';
	}
	else *s=' ';

//	*++s =temp_data/10000+0x30;
//	temp_data=temp_data%10000;     //取余运算
	*++s ='T';
	*++s ='e';
	*++s ='m';
	*++s ='p';
	*++s =':';	
	*++s =temp_data/1000+0x30;
	temp_data=temp_data%1000;     //取余运算

	*++s =temp_data/100+0x30;
	*++s ='.';
	temp_data=temp_data%100;     //取余运算
	*++s =temp_data/10+0x30;
	temp_data=temp_data%10;      //取余运算
	*++s =temp_data+0x30; 
	*++s ='C';	
	
	*++s ='\0'; 

}

//GPIO_Mode_AIN：模拟输入模式。
//GPIO_Mode_IN_FLOATING：浮空输入模式，无内部上下拉电阻。
//GPIO_Mode_IPD：下拉输入模式，内部上拉电阻被禁用。
//GPIO_Mode_IPU：上拉输入模式，内部下拉电阻被禁用。
//GPIO_Mode_Out_OD：开漏输出模式，可以与外部器件共享线路。
//GPIO_Mode_Out_PP：推挽输出模式，可以提供较高的输出电流。
//GPIO_Mode_AF_OD：开漏复用模式，可以与外部器件共享线路，用于特定的功能复用。
//GPIO_Mode_AF_PP：推挽复用模式，用于特定的功能复用，可以提供较高的输出电流。


//****************************************
//按键初始化函数
//****************************************
void  Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

	//KEY1 //KEY2
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO | KEY2_GPIO | KEY3_GPIO | KEY4_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}


//****************************************
//按键检测函数
//****************************************
void KeyDetection(void)  
{
	if (GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//按键消抖处理	
		if (GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO) == Bit_RESET && key1flag==0)
		{
			keyState=1;
			key1flag=1; //关门，不让按键状态因为电平发生多次赋值
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO) == Bit_SET)
	{
		Delay_ms(5);//按键消抖处理	
		key1flag=0; //开门，允许按键赋值
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//按键消抖处理	
		if (GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO) == Bit_RESET && key2flag==0)
		{
			keyState=2;
			key2flag=1; //关门，不让按键状态因为电平发生多次赋值
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO) == Bit_SET)
	{
		Delay_ms(5);//按键消抖处理	
		key2flag=0; //开门，允许按键赋值
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//按键消抖处理	
		if (GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO) == Bit_RESET && key3flag==0)
		{
			keyState=3;
			key3flag=1; //关门，不让按键状态因为电平发生多次赋值
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO) == Bit_SET)
	{
		Delay_ms(5);//按键消抖处理	
		key3flag=0; //开门，允许按键赋值
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO) == Bit_RESET)
	{
		Delay_ms(5);//按键消抖处理	
		if (GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO) == Bit_RESET && key4flag==0)
		{
			keyState=4;
			key4flag=1; //关门，不让按键状态因为电平发生多次赋值
		}
	}else if(GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO) == Bit_SET)
	{
		Delay_ms(5);//按键消抖处理	
		key4flag=0; //开门，允许按键赋值
	}
	
}


//****************************************
//主页面函数
//****************************************
void show_main(void)
{
	Vol =(float) 3.3*ADC_ConvertedValue/4096; 
	smoke = (Vol-0.27)/3.03*100;
	datapros(Ds18b20ReadTemp());
	sprintf(Dis_Smoke_Buf," Smoke:%.2f%% ",smoke);
	//		sprintf(Dis_Vol_Buf," Vol:%.2f ",Vol);
	//		sprintf(Dis_ADC_Buf,"AD_VA:%d   ",ADC_ConvertedValue);
	OLED_printf(DisplayData,D_temp);
	if(smoke>set_smoke)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_ResetBits(GPIOC, GPIO_Pin_15);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_15);
	}


	sscanf((const char *)DisplayData," Temp:%fC",&temp);
	if(temp > set_temp)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}

	//关闭蜂鸣器
	if(smoke<=set_smoke && temp<=set_temp)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_15);
	}

	OLED_ShowString(0,2,(u8 *)Dis_Smoke_Buf,16);
	//		OLED_ShowString(0,0,(u8 *)Dis_ADC_Buf,16);
	OLED_ShowString(0,4,(u8 *)DisplayData,16); 
	keyState = 0; //清除按键状态
}


//****************************************
//设置页面函数
//****************************************
void show_set(void)
{
	KeyDetection(); // 读取按键状态
	
	//设置烟雾报警线
	if(show_flag==1)
	{
		if(Handover >= 1)
		{
			OLED_Clear();
			Handover = 0;
		}
		
		if(keyState==1) //加键
		{
			set_smoke++;
			if(set_smoke>=99)
			set_smoke=99;
		}
		
		if(keyState==2) //减键
		{
			set_smoke--;
			if(set_smoke==9)
			{
				OLED_Clear();
			}
			if(set_smoke<=0)
			set_smoke=0;
		}
		OLED_ShowString(8,3,(u8 *)"SmokeAlarm:",16);
		OLED_ShowNumber(96,3,set_smoke,2,16);
		keyState = 0; //清除按键状态
		
	}
	
	
	//设置温度报警线
	if(show_flag==2)
	{
		if(Handover >= 1)
		{
			OLED_Clear();
			Handover = 0;
		}
		
		if(keyState==1)
		{
			set_temp++;
			if(set_temp>=99)
			set_temp=99;
		}
		
		if(keyState==2)
		{
			set_temp--;
			if(set_temp==9)
			{
				OLED_Clear();
			}
			if(set_temp<=0)
			set_temp=0;
		}
		 
		OLED_ShowString(8,2,(u8 *)"TempAlarm:",16);
		OLED_ShowNumber(88,2,set_temp,2,16);
		keyState = 0; //清除按键状态
	}	
}

//****************************************
//报警函数
//****************************************
void AlarmFunction(int ManualAlarm)
{
	//判断是否符合手动报警flag
		if(ManualAlarm == 1 )
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_15);  //低电平响
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);  //低电平亮
			GPIO_ResetBits(GPIOC, GPIO_Pin_15);  //低电平亮
			
		}
		else
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_15);  
			GPIO_SetBits(GPIOC, GPIO_Pin_13);  
			GPIO_SetBits(GPIOC, GPIO_Pin_15);  
			
		}
}








/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
	
	// 初始化
	BUZZER_GPIO_Config(GPIO_Pin_15); //B组引脚
	LED_GPIO_Config(GPIO_Pin_13); //C组引脚
	LED_GPIO_Config(GPIO_Pin_15); //C组引脚
	ADCx_Init(); //模式转换初始化
	DelayInit(); //初始化延迟函数
	OLED_Init(); //初始化OLED屏幕
	OLED_Clear(); //清屏
	DS18B20_OUT_GPIO_Config(); //初始化温度传感器引脚
	Key_GPIO_Config(); //初始化按键
	while (1)
	{
		KeyDetection(); // 读取按键状态
		
		if(keyState==3)//报警键
		{
			if(ManualAlarm == 0)
			{
				//手动报警键标志位 
				ManualAlarm = 1;
			}
			else
			{
				//手动报警键标志位 
				ManualAlarm = 0;
			}
			keyState = 0; //清除按键状态
		}
		
		if(keyState==4)//设置键
		{
			if(ManualAlarm != 1)
			{
				show_flag++; //主页面和设置页面切换标志位
				if(show_flag>=3)
				show_flag=0;
				Handover++; //切换页面标志位
			}
		}
		
		//满足show_flag = 0则显示主页面，ManualAlarm = 1则触发报警，Handover >= 1则表示切换过页面 
		if(show_flag == 0&&ManualAlarm != 1) //ManualAlarm=1时，可以得到手动报警值的固定值，即数值不再发生变化
		{
			
			if(Handover >= 1)
			{
				OLED_Clear();
				Handover = 0;
			}
			
			show_main(); //主页面
			

		}
		
		//满足show_flag == 1||show_flag == 2则显示报警阈值设置页面，1表示烟雾浓度设置页面和2表示温度设置页面
		if(show_flag == 1||show_flag == 2)
		{
			if(Handover >= 1)
			{
				OLED_Clear();
				Handover = 0;
			}
			show_set(); //设置页面
			
		}
		
		//报警函数
		AlarmFunction(ManualAlarm);
        
	}
	
}






