#include "stm32f4xx.h"
#include "bluetooth.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "MFRC522.h"
#include "oled.h"
#include "as608.h"
#include <string.h>
#include "mqtt.h"

#define  BLE_BUFFERSIZE  256						       //接收缓冲区的大小
static __IO char ble_buffer[BLE_BUFFERSIZE];   //作为蓝牙模块的接收缓冲区
static __IO int  ble_cnt=0;										 //作为蓝牙模块的接收计数器
static __IO uint8_t ble_recvflag = 0;			  	 //作为蓝牙模块的接收标志位

__IO char pwd_buf[16]="123456";  //作为密码的接收缓冲区  默认密码123456
__IO int  pwd_count = 0;				     	   //作为密码的接收计数器
extern __IO u8 PwdAlarmLimit;            //用来判断连续输入多少错误密码时报警
__IO char card_id[50][20]={{0}};         //用来存储50张卡号，长度最大为50位
__IO u8 card_cnt = 0;                    //门卡录入计数器
extern unsigned char BMP1[];
extern volatile uint32_t opencount;
extern volatile uint32_t alarmcount;


//配置蓝牙参数
void BLE_Config(u32 baud)
{
	USART3_Config(baud);
	
	Usart_SendString( USART3, "AT+NAMEWGLWGL\r\n");   //修改蓝牙名称
	delay_ms(10);
//	USART3_SendString("AT+BAUD4\r\n"); //设置蓝牙波特率 9600
//	delay_ms(10);
	Usart_SendString( USART3, "AT+RESET\r\n"); 				//参数重启生效
	delay_ms(10);
}

//修改密码
void ModPwd(void)
{
	//清空接收缓冲区
	memset((char *)pwd_buf,0,16);
	//清除接收计数器
	pwd_count = 0;
	OLED_Clear();
	OLED_ShowCHinese(0,2,99);	   //请输入您要修改的密码
	OLED_ShowCHinese(16,2,104);
	OLED_ShowCHinese(32,2,20);
	OLED_ShowCHinese(48,2,105);
	OLED_ShowCHinese(64,2,106);
	OLED_ShowCHinese(80,2,107);
	OLED_ShowCHinese(96,2,108);
	OLED_ShowCHinese(112,2,109);
	OLED_ShowCHinese(0,4,103);
	OLED_ShowCHinese(16,4,81);
	OLED_ShowString(0,6,"#",16);
	OLED_ShowCHinese(8,6,110); //清空
	OLED_ShowCHinese(24,6,111); 
	OLED_ShowString(40,6,"*",111);
	OLED_ShowCHinese(56,6,97); //确认
	OLED_ShowCHinese(72,6,98);
	u8 ClearFlag = 1;
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
			if(pwd_count < 16)
			{
				pwd_buf[pwd_count++]=KeyState;
				pwd_buf[pwd_count]='\0';
				OLED_ShowString(8,2,(u8 *)pwd_buf,16);
			}
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}
		else if(KeyState == '*')  //*表示确认键
		{
			//利用MQTT上传给阿里云
			mqtt_report_devices_status();
			OLED_Clear();
			OLED_ShowCHinese(16,3,103);
			OLED_ShowCHinese(32,3,81);
			OLED_ShowCHinese(48,3,107);
			OLED_ShowCHinese(64,3,108);
			OLED_ShowCHinese(80,3,95);
			OLED_ShowCHinese(96,3,96);
			delay_ms(500); //不能超过798
			delay_ms(500); //不能超过798
			OLED_Clear();
			OLED_ShowCHinese(0,3,0);	       //小
			OLED_ShowCHinese(18,3,1);	       //亮
			OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
			OLED_ShowCHinese(92,3,2);	       //门
			OLED_ShowCHinese(110,3,3);	     //锁
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
			break;
		}
		else if(KeyState == '#')  //#表示清除键
		{
			OLED_Clear();
			//清空接收缓冲区
			memset((char *)pwd_buf,0,16);
			//清除接收计数器
			pwd_count = 0;
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}	
	}		
}



//添加门卡
void AddKeyCard(void)
{
	u8 status = 1;
	u8 card_pydebuf[2];
	u8 card_numberbuf[5]; //最后一个字节是校验字节
	
	OLED_Clear();
	OLED_ShowCHinese(8,3,99);	    //请开始注册门卡
	OLED_ShowCHinese(24,3,89);	      
	OLED_ShowCHinese(40,3,100);	       
	OLED_ShowCHinese(56,3,101);	       
	OLED_ShowCHinese(72,3,102);	      
	OLED_ShowCHinese(88,3,2);	       
	OLED_ShowCHinese(104,3,94);	       
	while (1)
	{
		status = MFRC522_Request(0x52, card_pydebuf); //寻卡   S50卡的卡号是全球唯一的  卡内部有线圈  利用读取卡获取卡片信息  低频 13.56MHZ

		if (status == 0) //如果读到卡
		{
			if(card_cnt<50)
			{
				MFRC522_Anticoll(card_numberbuf); //防碰撞处理  把能量提供给卡号较大的卡片  可以得到卡号  卡号4字节
				sprintf((char *)card_id[card_cnt], "0x");
				for (int i = 0; i < 4; i++)
				{
					sprintf((char *)card_id[card_cnt] + 2 + 2 * i, "%02X", card_numberbuf[i]);
				}
				card_cnt++;
				OLED_Clear();
				GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
				delay_ms(100); //不能超过798
				GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
				OLED_ShowCHinese(16,2,101);	  //注册门卡成功     
				OLED_ShowCHinese(32,2,102);	      
				OLED_ShowCHinese(48,2,2);	       
				OLED_ShowCHinese(64,2,94);
				OLED_ShowCHinese(80,2,95);
				OLED_ShowCHinese(96,2,96);
				OLED_ShowString(24,4,(u8 *)card_id[card_cnt-1],16);
				delay_ms(500); //不能超过798
				delay_ms(500); //不能超过798
				
				OLED_Clear();
				OLED_ShowCHinese(0,3,0);	       //小
				OLED_ShowCHinese(18,3,1);	       //亮
				OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
				OLED_ShowCHinese(92,3,2);	       //门
				OLED_ShowCHinese(110,3,3);	     //锁
				status =1;
				break;
			}
			else
			{
				OLED_Clear();
				OLED_ShowString(0,2,"No more than 50 key cards",16);	
				delay_ms(500); //不能超过798
				delay_ms(500); //不能超过798
				OLED_Clear();
			}
			status =1;
		}
	}
}

//删除门卡
void DelKeyCard(void)
{
	int index = 0;
	OLED_Clear();
	OLED_ShowCHinese(32,0,54);	   //按顺序：删除门卡 
	OLED_ShowCHinese(48,0,55);
	OLED_ShowCHinese(64,0,2);
	OLED_ShowCHinese(80,0,94);
	OLED_ShowString(24,2,(u8 *)card_id[index],16);
	OLED_ShowString(16,4,"A+B-*",16);
	OLED_ShowCHinese(56,4,97); //确认
	OLED_ShowCHinese(88,4,98);
	while(1)
	{
		get_key(); //获取按键状态
		if(KeyState !=' ' && KeyState == 'A') //按下的键有效 A表示上翻 
		{
			if(index < card_cnt-1)
			{
				index++;
				OLED_ShowString(24,2,(u8 *)card_id[index],16);
			}
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}
		else if(KeyState !=' ' && KeyState == 'B')  //B表示下翻
		{
			//处理按键
			if(index >0)
			{
				index--;
				OLED_ShowString(24,2,(u8 *)card_id[index],16);
			}
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}
		else if(KeyState !=' ' && KeyState == '*')  //*表示确认键
		{
			if (index >= 0 && index < card_cnt)
			{
				// 将指定索引后面的数据向前移动一个位置
				for (int i = index; i < card_cnt - 1; i++) 
				{
					strcpy((char *)card_id[i], (char *)card_id[i + 1]);
				}
				// 将最后一个位置清空
				memset((char *)card_id[card_cnt - 1], 0, sizeof(card_id[card_cnt - 1]));
				// 更新计数器
				card_cnt--;
			}
			OLED_Clear();
			OLED_ShowCHinese(16,2,54);	   //按顺序：删除门卡成功
			OLED_ShowCHinese(32,2,55);
			OLED_ShowCHinese(48,2,2);
			OLED_ShowCHinese(64,2,94);
			OLED_ShowCHinese(80,2,95);
			OLED_ShowCHinese(96,2,96);
			OLED_ShowString(24,4,(u8 *)card_id[index],16);
			delay_ms(500); //不能超过798
			OLED_Clear();
			OLED_ShowCHinese(0,3,0);	       //小
			OLED_ShowCHinese(18,3,1);	       //亮
			OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
			OLED_ShowCHinese(92,3,2);	       //门
			OLED_ShowCHinese(110,3,3);	     //锁
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
			break;
		}
			
	}		
}

//判断蓝牙是否接收到数据并进行处理
void BlueRecvProcessing(void)
{
	if( 1 == ble_recvflag )
	{
		
		//处理接收缓冲区
		if(strstr((char *)ble_buffer,"OpenDoor#")!=NULL)
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
			GPIO_SetBits(GPIOF, GPIO_Pin_15); //门开
			opencount++;
			//利用MQTT上传给阿里云
			mqtt_report_devices_status();
			GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
			delay_ms(200); //不能超过798
			GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
			for(int i = 0 ; i<=8 ; i++)
			{
				delay_ms(500); //不能超过798
			}
			GPIO_ResetBits(GPIOF, GPIO_Pin_15); //门关
			PwdAlarmLimit=0; //复位，防止错误报警
			OLED_Clear(); 	//清屏
			OLED_ShowCHinese(0,3,0);	       //小
			OLED_ShowCHinese(18,3,1);	       //亮
			OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
			OLED_ShowCHinese(92,3,2);	       //门
			OLED_ShowCHinese(110,3,3);	     //锁
		}
		else if(strstr((char *)ble_buffer,"ModPwd#")!=NULL)
		{
			ModPwd(); //修改密码函数
			PwdAlarmLimit=0; //复位，防止错误报警
		}
		else if(strstr((char *)ble_buffer,"AddKeyCard#")!=NULL)
		{
			//添加门卡
			AddKeyCard();
			PwdAlarmLimit=0; //复位，防止错误报警
		}
		else if(strstr((char *)ble_buffer,"DelKeyCard#")!=NULL)
		{
			//删除门卡
			DelKeyCard();
			PwdAlarmLimit=0; //复位，防止错误报警
		}
		else if(strstr((char *)ble_buffer,"Add_FR#")!=NULL)
		{ 
			//录指纹
			Add_FR();
			PwdAlarmLimit=0; //复位，防止错误报警
			OLED_Clear(); 	//清屏
			OLED_ShowCHinese(0,3,0);	       //小
			OLED_ShowCHinese(18,3,1);	       //亮
			OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
			OLED_ShowCHinese(92,3,2);	       //门
			OLED_ShowCHinese(110,3,3);	     //锁
			
		}
		else if(strstr((char *)ble_buffer,"Del_FR#")!=NULL)
		{ 
			//删除指纹
			Del_FR();
			PwdAlarmLimit=0; //复位，防止错误报警
			OLED_Clear(); 	//清屏
			OLED_ShowCHinese(0,3,0);	       //小
			OLED_ShowCHinese(18,3,1);	       //亮
			OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
			OLED_ShowCHinese(92,3,2);	       //门
			OLED_ShowCHinese(110,3,3);	     //锁
			
		}
		else
		{
			PwdAlarmLimit++;
			if(PwdAlarmLimit==8) //错误密码连续超过10次就报警
			{
				alarmcount++;
				//利用MQTT上传给阿里云
				mqtt_report_devices_status();
				for(int i=0;i<25;i++)
				{
					GPIO_SetBits(GPIOF, GPIO_Pin_8); //蜂鸣器响
					delay_ms(50); //不能超过798
					GPIO_ResetBits(GPIOF, GPIO_Pin_8); //蜂鸣器关
					delay_ms(50); //不能超过798
				}
				PwdAlarmLimit=0; //复位，防止错误报警
			}
		}
			
		
		//清除接收标志位
		ble_recvflag = 0;
		
		//清空接收缓冲区
		memset((char *)ble_buffer,0,BLE_BUFFERSIZE);
		
		//清除接收计数器
		ble_cnt = 0;
		
	}
}

//中断服务函数 接收蓝牙数据
void USART3_IRQHandler(void)
{
	
	//判断中断是否触发
	if( USART_GetITStatus(USART3, USART_IT_RXNE) == SET )
	{
		//判断接收缓冲区是否满了
		if(ble_cnt < BLE_BUFFERSIZE)
		{
			ble_buffer[ble_cnt++] = USART_ReceiveData(USART3); //一次只能接收1个字节
			
			//说明数据接收完成
			if(ble_buffer[ble_cnt-1] == '#')
			{
				ble_recvflag = 1;
			}
		}
		
	}
}



