#ifndef __MATRIX_KEY_H
#define	__MATRIX_KEY_H
#include "stm32f4xx.h"

//按键值 即按键状态
extern char KeyState;

// 初始化键盘
void Key_Init(void);
// 获取键盘数据
void get_key(void);

#endif 

