#ifndef __USART_H
#define __USART_H
#include "stm32f4xx.h"
#include "stdio.h"
 
#define USART2_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART2_MAX_SEND_LEN		400					//最大发送缓存字节数
#define USART2_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern vu16 USART2_RX_STA;   						//接收数据状态

void u2_printf(char* fmt,...);

//USART2的初始化
void USART2_Config(u32 baud);

//USART3的初始化
void USART3_Config(u32 baud);

//发送一个字节
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);

//发送字符串的函数
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);

//发送一个16位数
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);

///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f);

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f);

#endif
