#ifndef __MATRIX_KEY_H
#define	__MATRIX_KEY_H
#include "stm32f4xx.h"

//����ֵ ������״̬
extern char KeyState;

// ��ʼ������
void Key_Init(void);
// ��ȡ��������
void get_key(void);

#endif 

