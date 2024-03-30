#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H
#include "stm32f4xx.h" 

//判断蓝牙是否接收到数据并进行处理
void BlueRecvProcessing(void);
//配置蓝牙参数
void BLE_Config(u32 baud);

extern __IO char pwd_buf[16];
extern __IO char card_id[50][20];         //用来存储50张卡号，长度最大为20位
extern __IO u8 card_cnt; //门卡录入计数器
#endif

