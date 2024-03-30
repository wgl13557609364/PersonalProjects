#ifndef __USART_H
#define __USART_H
#include "stm32f4xx.h"
#include "stdio.h"
 
#define USART2_MAX_RECV_LEN		400					//�����ջ����ֽ���
#define USART2_MAX_SEND_LEN		400					//����ͻ����ֽ���
#define USART2_RX_EN 			1					//0,������;1,����.

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//���ջ���,���USART2_MAX_RECV_LEN�ֽ�
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
extern vu16 USART2_RX_STA;   						//��������״̬

extern uint8_t Rx1Counter;
extern uint8_t Rx1Data;
extern uint8_t Rx1Buffer[256];


extern uint8_t	 Tx4Buffer[512];
extern volatile uint32_t	Rx4Counter;
extern volatile uint8_t	Rx4Data;
extern volatile uint8_t	Rx4End;
extern volatile uint8_t	Rx4Buffer[512];



void u2_printf(char* fmt,...);

//����1�ĳ�ʼ��
void USART1_Config(u32 baud);

//USART2�ĳ�ʼ��
void USART2_Config(u32 baud);

//USART3�ĳ�ʼ��
void USART3_Config(u32 baud);

//����4�ĳ�ʼ��
void UART4_Config(u32 baud);

//����3�ĳ�ʼ��
void USART3_Config(u32 baud);

//�����ַ����ĺ���
void USART3_SendString(char *str);

//�����ַ����ĺ���
void UART4_SendString(char *str);

/*****************  ����һ���ֽ� **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);

/*****************  ����ָ�����ȵ��ֽ� **********************/
void Usart_SendBytes(USART_TypeDef * pUSARTx, uint8_t *buf,uint32_t len);

/****************** ����8λ������ ************************/
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num);

/*****************  �����ַ��� **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);

/*****************  ����һ��16λ�� **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f);

#endif
