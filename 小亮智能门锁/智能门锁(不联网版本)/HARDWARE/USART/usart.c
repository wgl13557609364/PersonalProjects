#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "timer.h"
#include "stdarg.h"	

//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			  //发送缓冲,最大USART2_MAX_SEND_LEN字节

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART2_RX_STA=0;   	

//USART2的初始化
void USART2_Config(u32 baud)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  //1.打开GPIO端口  PA2 PA3  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  //2.打开串口时钟  USART2 -- APB1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  
  //3.选择引脚的复用功能
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2  , GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3  , GPIO_AF_USART2);
  
  //4.配置GPIO引脚参数并初始化
  GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;			 			//复用模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 			//输出速度
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		 			//推挽复用
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;			 			//上拉电阻
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3;	//引脚编号
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	//5.配置USART1的参数并初始化
  USART_InitStructure.USART_BaudRate 		= baud;										//波特率
  USART_InitStructure.USART_WordLength 	= USART_WordLength_8b;		//数据位
  USART_InitStructure.USART_StopBits 		= USART_StopBits_1;				//停止位
  USART_InitStructure.USART_Parity 			= USART_Parity_No;				//检验位
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
  USART_Init(USART2, &USART_InitStructure);
  
  //6.配置中断参数并初始化
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;								//中断通道编号
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;				//抢占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;							//子优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//打开中断通道
  NVIC_Init(&NVIC_InitStructure);
  
	//7.选择中断源   接收到数据则触发中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

  //8.打开USART2
  USART_Cmd(USART2, ENABLE);
	
	TIM7_Int_Init(100-1,8400-1);		//10ms中断
	USART2_RX_STA=0;		//清零
	TIM_Cmd(TIM7,DISABLE);			//关闭定时器7
}



void USART2_IRQHandler(void)
{
	u8 res;	      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART2);		 
		if((USART2_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM7,0);//计数器清空          				//计数器清空
				if(USART2_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM_Cmd(TIM7,ENABLE);//使能定时器7
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  				 											 
}   


//USART3的初始化
void USART3_Config(u32 baud)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  //1.打开GPIO端口时钟  PB10 PB11 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  //2.打开串口时钟  USART3 -- APB1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  
  //3.选择引脚的复用功能
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10 , GPIO_AF_USART3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11 , GPIO_AF_USART3);
  
  //4.配置GPIO引脚参数并初始化
  GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;			 			//复用模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 			//输出速度
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		 			//推挽复用
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;			 			//上拉电阻
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10|GPIO_Pin_11;	//引脚编号
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
	//5.配置USART3的参数并初始化
  USART_InitStructure.USART_BaudRate 		= baud;										//波特率
  USART_InitStructure.USART_WordLength 	= USART_WordLength_8b;		//数据位
  USART_InitStructure.USART_StopBits 		= USART_StopBits_1;				//停止位
  USART_InitStructure.USART_Parity 			= USART_Parity_No;				//检验位
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
  USART_Init(USART3, &USART_InitStructure);
  
  //6.配置中断参数并初始化
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;								//中断通道编号
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;				//抢占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;							//子优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//打开中断通道
  NVIC_Init(&NVIC_InitStructure);
  
	//7.选择中断源   接收到数据则触发中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

  //8.打开USART3
  USART_Cmd(USART3, ENABLE);
	
}



/*****************  发送一个字节 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/****************** 发送8位的数组 ************************/
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)
{
  uint8_t i;
	
	for(i=0; i<num; i++)
  {
	    /* 发送一个字节数据到USART */
	    Usart_SendByte(pUSARTx,array[i]);	
  
  }
	/* 等待发送完成 */
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}

/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(USART2, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(USART2);
}




