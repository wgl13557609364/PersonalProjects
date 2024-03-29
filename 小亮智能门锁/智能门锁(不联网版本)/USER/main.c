/***********************************************************************************
* @file    main.c 
* @author  
* @version V1
* @date    
* @brief   
***********************************************************************************/

#include "stm32f4xx.h"
#include "bluetooth.h"
#include "MFRC522.h"
#include "Relay.h"
#include "oled.h"
#include "beep.h"
#include "key.h"
#include "timer.h"
#include "as608.h"
#include "delay.h"
#include "usart.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "bmp.h"

__IO char PwdIn_Buf[16] = {0}; //存储开门密码
__IO int PwdIn_cnt = 0;        //开门密码计数器
__IO u8 PwdAlarmLimit = 0;     //用来判断连续输入多少错误密码时报警
__IO u8 status=1;
__IO u8 card_pydebuf[2];
__IO u8 card_numberbuf[5];  //最后一个字节是校验字节

void PasswordInput(void)
{
	if(KeyState !=' ' && KeyState != '*' && KeyState != '#') //按下的键有效 根据需要在这里执行相应的操作 
	{
		OLED_Clear();
		u8 ClearFlag = 0; //清屏标志
		//清空接收缓冲区
		memset((char *)PwdIn_Buf,0,16);
		//清除接收计数器
		PwdIn_cnt = 0;
		PwdIn_Buf[PwdIn_cnt++]=KeyState;
		PwdIn_Buf[PwdIn_cnt]='\0';
		OLED_ShowString(8,2,(u8 *)PwdIn_Buf,16);
		// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
		KeyState = ' ';
		while(1)
		{
			get_key(); //获取按键状态
			if(KeyState !=' ' && KeyState != '*' && KeyState != '#') //按下的键有效 根据需要在这里执行相应的操作 
			{
				if(ClearFlag) //清屏
				{
					OLED_Clear();
					ClearFlag=0;
				}
				//处理按键
				if(PwdIn_cnt < 16)
				{
					PwdIn_Buf[PwdIn_cnt++]=KeyState;
					PwdIn_Buf[PwdIn_cnt]='\0';
					OLED_ShowString(8,2,(u8 *)PwdIn_Buf,16);
				}
				// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
				KeyState = ' ';
			}
			else if(KeyState == '*')  //*表示确认键
			{
				OLED_Clear();
				if(strstr((char *)PwdIn_Buf,(char *)pwd_buf)!=NULL)
				{
					OLED_ShowCHinese(32,2,2);	   //按顺序：门已打开 
					OLED_ShowCHinese(48,2,87);
					OLED_ShowCHinese(64,2,88);
					OLED_ShowCHinese(80,2,89);
					OLED_ShowCHinese(32,4,90);	   //按顺序：欢迎光临 
					OLED_ShowCHinese(48,4,91);
					OLED_ShowCHinese(64,4,92);
					OLED_ShowCHinese(80,4,93);
					GPIO_SetBits(GPIOF, GPIO_Pin_15); //门开
					GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
					delay_ms(200); //不能超过798
					GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
					for(int i = 0 ; i<=8 ; i++)
					{
						delay_ms(500); //不能超过798
					}
					GPIO_ResetBits(GPIOF, GPIO_Pin_15); //门关
					PwdAlarmLimit=0; //复位，防止错误报警
					OLED_Clear();
					OLED_ShowCHinese(0,3,0);	       //小
					OLED_ShowCHinese(18,3,1);	       //亮
					OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
					OLED_ShowCHinese(92,3,2);	       //门
					OLED_ShowCHinese(110,3,3);	     //锁
				}
				else
				{
					//密码错误
					OLED_ShowCHinese(32,3,103);	       
					OLED_ShowCHinese(48,3,81);	       
					OLED_ShowCHinese(64,3,9);	      
					OLED_ShowCHinese(80,3,10);	 
					PwdAlarmLimit++;
					if(PwdAlarmLimit==8) //错误密码连续超过5次就报警
					{
						for(int i=0;i<25;i++)
						{
							GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
							delay_ms(50); //不能超过798
							GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
							delay_ms(50); //不能超过798
						}
						PwdAlarmLimit=0; //复位，防止错误报警
					}
					for(int i=0;i<4;i++)
					{
						delay_ms(500); //不能超过798
					}
					OLED_Clear();
					OLED_ShowCHinese(0,3,0);	       //小
					OLED_ShowCHinese(18,3,1);	       //亮
					OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
					OLED_ShowCHinese(92,3,2);	       //门
					OLED_ShowCHinese(110,3,3);	     //锁
				}
				
				// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
				KeyState = ' ';
				break;
			}
			else if(KeyState == '#')  //#表示清除键
			{
				OLED_Clear();
				//清空接收缓冲区
				memset((char *)PwdIn_cnt,0,16);
				//清除接收计数器
				PwdIn_cnt = 0;
				// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
				KeyState = ' ';
				OLED_ShowCHinese(0,3,0);	       //小
				OLED_ShowCHinese(18,3,1);	       //亮
				OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
				OLED_ShowCHinese(92,3,2);	       //门
				OLED_ShowCHinese(110,3,3);	     //锁
				
				ClearFlag=1;
			}	
		}
	}
}


//读卡开门函数
void ReadCardOpen(void)
{
	status=MFRC522_Request(0x52, (u8 *)card_pydebuf);			//寻卡   S50卡的卡号是全球唯一的  卡内部有线圈  利用读取卡获取卡片信息  低频 13.56MHZ  
	
	if(status==0)		//如果读到卡
	{
		MFRC522_Anticoll((u8 *)card_numberbuf);			//防碰撞处理  把能量提供给卡号较大的卡片  可以得到卡号  卡号4字节
		char card_tmpid[20]="0x";
		for(int i=0;i<4;i++)
		{
			sprintf(card_tmpid+2+2*i,"%02X",card_numberbuf[i]);
		}
		for(int i=0;i<card_cnt;i++)
		{
			if(strstr((char *)card_tmpid,(char*)card_id[i])!=NULL) //比对
			{
				OLED_Clear(); 	//清屏
				OLED_ShowCHinese(32,2,2);	   //按顺序：门已打开 
				OLED_ShowCHinese(48,2,87);
				OLED_ShowCHinese(64,2,88);
				OLED_ShowCHinese(80,2,89);
				OLED_ShowCHinese(32,4,90);	   //按顺序：欢迎光临 
				OLED_ShowCHinese(48,4,91);
				OLED_ShowCHinese(64,4,92);
				OLED_ShowCHinese(80,4,93);
				OLED_ShowString(32,6,(u8 *)card_tmpid,16);
				GPIO_SetBits(GPIOF, GPIO_Pin_15);//门开
				GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
				delay_ms(200); //不能超过798
				GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
				for(int i = 0 ; i<=8 ; i++)
				{
					delay_ms(500); //不能超过798
				}
				GPIO_ResetBits(GPIOF, GPIO_Pin_15);//门关
				PwdAlarmLimit=0; //复位，防止错误报警
				OLED_Clear(); 	//清屏
				OLED_ShowCHinese(0,3,0);	       //小
				OLED_ShowCHinese(18,3,1);	       //亮
				OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
				OLED_ShowCHinese(92,3,2);	       //门
				OLED_ShowCHinese(110,3,3);	     //锁
				status=1;
				return; //开完门跳出比对
			}
		}
		OLED_Clear(); 	//清屏
		OLED_ShowCHinese(16,3,2);	    //门卡没有注册   
		OLED_ShowCHinese(32,3,94);	  
		OLED_ShowCHinese(48,3,15);	     
		OLED_ShowCHinese(64,3,16);	       
		OLED_ShowCHinese(80,3,101);	       
		OLED_ShowCHinese(96,3,102);	      
		 
		delay_ms(500); //不能超过798
		OLED_Clear(); 	//清屏
		OLED_ShowCHinese(0,3,0);	       //小
		OLED_ShowCHinese(18,3,1);	       //亮
		OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
		OLED_ShowCHinese(92,3,2);	       //门
		OLED_ShowCHinese(110,3,3);	     //锁

		PwdAlarmLimit++;
		if(PwdAlarmLimit==8) //错误密码连续超过5次就报警
		{
			OLED_Clear(); 	//清屏
			OLED_ShowCHinese(16,3,2);	    //门卡没有注册   
			OLED_ShowCHinese(32,3,94);	  
			OLED_ShowCHinese(48,3,15);	     
			OLED_ShowCHinese(64,3,16);	       
			OLED_ShowCHinese(80,3,101);	       
			OLED_ShowCHinese(96,3,102);	
			for(int i=0;i<25;i++)
			{
				GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
				delay_ms(50); //不能超过798
				GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
				delay_ms(50); //不能超过798
				
			}
			OLED_Clear(); 	//清屏
			OLED_ShowCHinese(0,3,0);	       //小
			OLED_ShowCHinese(18,3,1);	       //亮
			OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
			OLED_ShowCHinese(92,3,2);	       //门
			OLED_ShowCHinese(110,3,3);	     //锁
			PwdAlarmLimit=0; //复位，防止错误报警
		}
		//恢复读卡状态
		status=1;
	}
}


//程序入口
int main()
{
	//1.硬件初始化
	BLE_Config(9600); //蓝牙初始化
	MFRC522_Initializtion(); //RFID的初始化
	Relay_Config(); //继电器初始化
	Beep_Config();  //蜂鸣器初始化
	Key_Init(); //键盘初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	//USART2的初始化
	USART2_Config(57600);
	OLED_Init();		//初始化OLED  
	OLED_Clear();  //清屏

	OLED_ShowCHinese(0,3,0);	       //小
	OLED_ShowCHinese(18,3,1);	       //亮
	OLED_DrawBMP(46,1,81,7,BMP1);    //显示图片
	OLED_ShowCHinese(92,3,2);	       //门
	OLED_ShowCHinese(110,3,3);	     //锁
	
  while(1)
	{
		//读取按键
		get_key();
		//读取输入密码并判断是否正确
		PasswordInput();
		
		//刷指纹
		press_FR();
		//读卡开门函数
		ReadCardOpen();
		
		//判断蓝牙是否接收到数据并进行处理
		BlueRecvProcessing();
		
	}
}








