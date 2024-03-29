#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "usart.h"
#include "as608.h"
#include "oled.h"
#include "key.h"

u32 AS608Addr = 0XFFFFFFFF; //默认
char str2[6] = {0};
extern unsigned char BMP1[];
extern __IO u8 PwdAlarmLimit;            //用来判断连续输入多少错误时报警
//串口发送一个字节
static void MYUSART_SendData(u8 data)
{
  while((USART2->SR & 0X40) == 0);
  USART2->DR = data;
}
//发送包头
static void SendHead(void)
{
  MYUSART_SendData(0xEF);
  MYUSART_SendData(0x01);
}
//发送地址
static void SendAddr(void)
{
  MYUSART_SendData(AS608Addr >> 24);
  MYUSART_SendData(AS608Addr >> 16);
  MYUSART_SendData(AS608Addr >> 8);
  MYUSART_SendData(AS608Addr);
}
//发送包标识,
static void SendFlag(u8 flag)
{
  MYUSART_SendData(flag);
}
//发送包长度
static void SendLength(int length)
{
  MYUSART_SendData(length >> 8);
  MYUSART_SendData(length);
}
//发送指令码
static void Sendcmd(u8 cmd)
{
  MYUSART_SendData(cmd);
}
//发送校验和
static void SendCheck(u16 check)
{
  MYUSART_SendData(check >> 8);
  MYUSART_SendData(check);
}
//判断中断接收的数组有没有应答包
//waittime为等待中断接收数据的时间（单位1ms）
//返回值：数据包首地址
static u8 *JudgeStr(u16 waittime)
{
  char *data;
  u8 str[8];
  str[0] = 0xef;
  str[1] = 0x01;
  str[2] = AS608Addr >> 24;
  str[3] = AS608Addr >> 16;
  str[4] = AS608Addr >> 8;
  str[5] = AS608Addr;
  str[6] = 0x07;
  str[7] = '\0';
  USART2_RX_STA = 0;
  while(--waittime)
  {
    delay_ms(1);
    if(USART2_RX_STA & 0X8000) //接收到一次数据
    {
      USART2_RX_STA = 0;
      data = strstr((const char*)USART2_RX_BUF, (const char*)str);
      if(data)
        return (u8*)data;
    }
  }
  return 0;
}
//录入图像 PS_GetImage
//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。
//模块返回确认字
u8 PS_GetImage(void)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x03);
  Sendcmd(0x01);
  temp =  0x01 + 0x03 + 0x01;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//生成特征 PS_GenChar
//功能:将ImageBuffer中的原始图像生成指纹特征文件存于CharBuffer1或CharBuffer2
//参数:BufferID --> charBuffer1:0x01	charBuffer1:0x02
//模块返回确认字
u8 PS_GenChar(u8 BufferID)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x04);
  Sendcmd(0x02);
  MYUSART_SendData(BufferID);
  temp = 0x01 + 0x04 + 0x02 + BufferID;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//精确比对两枚指纹特征 PS_Match
//功能:精确比对CharBuffer1 与CharBuffer2 中的特征文件
//模块返回确认字
u8 PS_Match(void)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x03);
  Sendcmd(0x03);
  temp = 0x01 + 0x03 + 0x03;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//搜索指纹 PS_Search
//功能:以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库.若搜索到，则返回页码。
//参数:  BufferID @ref CharBuffer1	CharBuffer2
//说明:  模块返回确认字，页码（相配指纹模板）
u8 PS_Search(u8 BufferID, u16 StartPage, u16 PageNum, SearchResult *p)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x08);
  Sendcmd(0x04);
  MYUSART_SendData(BufferID);
  MYUSART_SendData(StartPage >> 8);
  MYUSART_SendData(StartPage);
  MYUSART_SendData(PageNum >> 8);
  MYUSART_SendData(PageNum);
  temp = 0x01 + 0x08 + 0x04 + BufferID
         + (StartPage >> 8) + (u8)StartPage
         + (PageNum >> 8) + (u8)PageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    p->pageID   = (data[10] << 8) + data[11];
    p->mathscore = (data[12] << 8) + data[13];
  }
  else
    ensure = 0xff;
  return ensure;
}
//合并特征（生成模板）PS_RegModel
//功能:将CharBuffer1与CharBuffer2中的特征文件合并生成 模板,结果存于CharBuffer1与CharBuffer2
//说明:  模块返回确认字
u8 PS_RegModel(void)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x03);
  Sendcmd(0x05);
  temp = 0x01 + 0x03 + 0x05;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//储存模板 PS_StoreChar
//功能:将 CharBuffer1 或 CharBuffer2 中的模板文件存到 PageID 号flash数据库位置。
//参数:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID（指纹库位置号）
//说明:  模块返回确认字
u8 PS_StoreChar(u8 BufferID, u16 PageID)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x06);
  Sendcmd(0x06);
  MYUSART_SendData(BufferID);
  MYUSART_SendData(PageID >> 8);
  MYUSART_SendData(PageID);
  temp = 0x01 + 0x06 + 0x06 + BufferID
         + (PageID >> 8) + (u8)PageID;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//删除模板 PS_DeletChar
//功能:  删除flash数据库中指定ID号开始的N个指纹模板
//参数:  PageID(指纹库模板号)，N删除的模板个数。
//说明:  模块返回确认字
u8 PS_DeletChar(u16 PageID, u16 N)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x07);
  Sendcmd(0x0C);
  MYUSART_SendData(PageID >> 8);
  MYUSART_SendData(PageID);
  MYUSART_SendData(N >> 8);
  MYUSART_SendData(N);
  temp = 0x01 + 0x07 + 0x0C
         + (PageID >> 8) + (u8)PageID
         + (N >> 8) + (u8)N;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//清空指纹库 PS_Empty
//功能:  删除flash数据库中所有指纹模板
//参数:  无
//说明:  模块返回确认字
u8 PS_Empty(void)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x03);
  Sendcmd(0x0D);
  temp = 0x01 + 0x03 + 0x0D;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//写系统寄存器 PS_WriteReg
//功能:  写模块寄存器
//参数:  寄存器序号RegNum:4\5\6
//说明:  模块返回确认字
u8 PS_WriteReg(u8 RegNum, u8 DATA)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x05);
  Sendcmd(0x0E);
  MYUSART_SendData(RegNum);
  MYUSART_SendData(DATA);
  temp = RegNum + DATA + 0x01 + 0x05 + 0x0E;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  if(ensure == 0)
    printf("\r\nSet the parameters successfully!");
  else
    EnsureMessage(ensure);
  return ensure;
}
//读系统基本参数 PS_ReadSysPara
//功能:  读取模块的基本参数（波特率，包大小等)
//参数:  无
//说明:  模块返回确认字 + 基本参数（16bytes）
u8 PS_ReadSysPara(SysPara *p)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x03);
  Sendcmd(0x0F);
  temp = 0x01 + 0x03 + 0x0F;
  SendCheck(temp);
  data = JudgeStr(1000);
  if(data)
  {
    ensure = data[9];
    p->PS_max = (data[14] << 8) + data[15];
    p->PS_level = data[17];
    p->PS_addr = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
    p->PS_size = data[23];
    p->PS_N = data[25];
  }
  else
     ensure = 0xff;
  if(ensure == 0x00)
  {
    printf("\r\nMaximum fingerprint capacity of module=%d", p->PS_max);
    printf("\r\nContrast grade=%d", p->PS_level);
    printf("\r\nAddress=%x", p->PS_addr);
    printf("\r\nBaud rate=%d", p->PS_N * 9600);
  }
  else
    EnsureMessage(ensure);
  return ensure;
}
//设置模块地址 PS_SetAddr
//功能:  设置模块地址
//参数:  PS_addr
//说明:  模块返回确认字
u8 PS_SetAddr(u32 PS_addr)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x07);
  Sendcmd(0x15);
  MYUSART_SendData(PS_addr >> 24);
  MYUSART_SendData(PS_addr >> 16);
  MYUSART_SendData(PS_addr >> 8);
  MYUSART_SendData(PS_addr);
  temp = 0x01 + 0x07 + 0x15
         + (u8)(PS_addr >> 24) + (u8)(PS_addr >> 16)
         + (u8)(PS_addr >> 8) + (u8)PS_addr;
  SendCheck(temp);
  AS608Addr = PS_addr; //发送完指令，更换地址
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  AS608Addr = PS_addr;
  if(ensure == 0x00)
    printf("\r\nAddress set successfully!");
  else
    EnsureMessage(ensure);
  return ensure;
}
//功能： 模块内部为用户开辟了256bytes的FLASH空间用于存用户记事本,
//	该记事本逻辑上被分成 16 个页。
//参数:  NotePageNum(0~15),Byte32(要写入内容，32个字节)
//说明:  模块返回确认字
u8 PS_WriteNotepad(u8 NotePageNum, u8 *Byte32)
{
  u16 temp;
  u8  ensure, i;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(36);
  Sendcmd(0x18);
  MYUSART_SendData(NotePageNum);
  for(i = 0; i < 32; i++)
  {
    MYUSART_SendData(Byte32[i]);
    temp += Byte32[i];
  }
  temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//读记事PS_ReadNotepad
//功能：  读取FLASH用户区的128bytes数据
//参数:  NotePageNum(0~15)
//说明:  模块返回确认字+用户信息
u8 PS_ReadNotepad(u8 NotePageNum, u8 *Byte32)
{
  u16 temp;
  u8  ensure, i;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x04);
  Sendcmd(0x19);
  MYUSART_SendData(NotePageNum);
  temp = 0x01 + 0x04 + 0x19 + NotePageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    for(i = 0; i < 32; i++)
    {
      Byte32[i] = data[10 + i];
    }
  }
  else
    ensure = 0xff;
  return ensure;
}
//高速搜索PS_HighSpeedSearch
//功能：以 CharBuffer1或CharBuffer2中的特征文件高速搜索整个或部分指纹库。
//		  若搜索到，则返回页码,该指令对于的确存在于指纹库中 ，且登录时质量
//		  很好的指纹，会很快给出搜索结果。
//参数:  BufferID， StartPage(起始页)，PageNum（页数）
//说明:  模块返回确认字+页码（相配指纹模板）
u8 PS_HighSpeedSearch(u8 BufferID, u16 StartPage, u16 PageNum, SearchResult *p)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x08);
  Sendcmd(0x1b);
  MYUSART_SendData(BufferID);
  MYUSART_SendData(StartPage >> 8);
  MYUSART_SendData(StartPage);
  MYUSART_SendData(PageNum >> 8);
  MYUSART_SendData(PageNum);
  temp = 0x01 + 0x08 + 0x1b + BufferID
         + (StartPage >> 8) + (u8)StartPage
         + (PageNum >> 8) + (u8)PageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    p->pageID 	= (data[10] << 8) + data[11];
    p->mathscore = (data[12] << 8) + data[13];
  }
  else
    ensure = 0xff;
  return ensure;
}
//读有效模板个数 PS_ValidTempleteNum
//功能：读有效模板个数
//参数: 无
//说明: 模块返回确认字+有效模板个数ValidN
u8 PS_ValidTempleteNum(u16 *ValidN)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//命令包标识
  SendLength(0x03);
  Sendcmd(0x1d);
  temp = 0x01 + 0x03 + 0x1d;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    *ValidN = (data[10] << 8) + data[11];
  }
  else
    ensure = 0xff;

  if(ensure == 0x00)
  {
    printf("\r\nNumber of effective fingerprints=%d", (data[10] << 8) + data[11]);
  }
  else
    EnsureMessage(ensure);
  return ensure;
}
//与AS608握手 PS_HandShake
//参数: PS_Addr地址指针
//说明: 模块返新地址（正确地址）
u8 PS_HandShake(u32 *PS_Addr)
{
  SendHead();
  SendAddr();
  MYUSART_SendData(0X01);
  MYUSART_SendData(0X00);
  MYUSART_SendData(0X00);
  delay_ms(200);
  if(USART2_RX_STA & 0X8000) //接收到数据
  {
    if(//判断是不是模块返回的应答包
      USART2_RX_BUF[0] == 0XEF
      && USART2_RX_BUF[1] == 0X01
      && USART2_RX_BUF[6] == 0X07
    )
    {
      *PS_Addr = (USART2_RX_BUF[2] << 24) + (USART2_RX_BUF[3] << 16)
                 + (USART2_RX_BUF[4] << 8) + (USART2_RX_BUF[5]);
      USART2_RX_STA = 0;
      return 0;
    }
    USART2_RX_STA = 0;
  }
  return 1;
}
//模块应答包确认码信息解析
//功能：解析确认码错误信息返回信息
//参数: ensure
void EnsureMessage(u8 ensure)
{
  switch(ensure)
  {
    case  0x00:
			OLED_ShowString(56, 3, "OK",16);
			break;
		case  0x01:
			OLED_ShowCHinese(0,3,4);	       //数
			OLED_ShowCHinese(16,3,5);	       //据
			OLED_ShowCHinese(32,3,6);	       //包
			OLED_ShowCHinese(48,3,7);	       //接
			OLED_ShowCHinese(64,3,8);	       //收
			OLED_ShowCHinese(80,3,9);	       //错
			OLED_ShowCHinese(96,3,10);	     //误			
			break;
		case  0x02:
			OLED_ShowCHinese(0,3,11);	       //传
			OLED_ShowCHinese(16,3,12);	     //感
			OLED_ShowCHinese(32,3,13);	     //器
			OLED_ShowCHinese(48,3,14);	     //上
			OLED_ShowCHinese(64,3,15);	     //没
			OLED_ShowCHinese(80,3,16);	     //有
			OLED_ShowCHinese(96,3,17);	     //手
			OLED_ShowCHinese(112,3,18);	     //指
			break;
		case  0x03:
			OLED_ShowCHinese(0,3,19);	    //按顺序：录入指纹图像失败   
			OLED_ShowCHinese(16,3,20);	    
			OLED_ShowCHinese(32,3,21);	     
			OLED_ShowCHinese(48,3,22);	     
			OLED_ShowCHinese(64,3,23);	     
			OLED_ShowCHinese(80,3,24);	     
			OLED_ShowCHinese(96,3,25);	     
			OLED_ShowCHinese(112,3,26);	     
			break;
		case  0x04:
			OLED_ShowCHinese(0,3,21);	    //按顺序：指纹太干或太淡  
			OLED_ShowCHinese(16,3,22);	    
			OLED_ShowCHinese(32,3,29);	     
			OLED_ShowCHinese(48,3,27);	     
			OLED_ShowCHinese(64,3,28);	     
			OLED_ShowCHinese(80,3,29);	     
			OLED_ShowCHinese(96,3,30);	          
			break;
		case  0x05:
			OLED_ShowCHinese(0,3,21);	    //按顺序：指纹太湿或太糊  
			OLED_ShowCHinese(16,3,22);	    
			OLED_ShowCHinese(32,3,29);	     
			OLED_ShowCHinese(48,3,27);	     
			OLED_ShowCHinese(64,3,31);	     
			OLED_ShowCHinese(80,3,29);	     
			OLED_ShowCHinese(96,3,32);	       
			break;
		case  0x06:
			OLED_ShowCHinese(0,3,21);	   //按顺序：指纹图像太乱  
			OLED_ShowCHinese(16,3,22);	    
			OLED_ShowCHinese(32,3,23);	     
			OLED_ShowCHinese(48,3,24);	     	     
			OLED_ShowCHinese(64,3,29);	     
			OLED_ShowCHinese(80,3,33);	  
			break;
		case  0x07:
			OLED_ShowCHinese(0,3,21);	   //按顺序：指纹特征点太少  
			OLED_ShowCHinese(16,3,22);	    
			OLED_ShowCHinese(32,3,34);	     
			OLED_ShowCHinese(48,3,35);	     	     
			OLED_ShowCHinese(64,3,36);	     
			OLED_ShowCHinese(80,3,29);	
			OLED_ShowCHinese(96,3,37);
			break;
		case  0x08:
			OLED_ShowCHinese(0,3,21);	   //按顺序：指纹不匹配  
			OLED_ShowCHinese(16,3,22);	    
			OLED_ShowCHinese(32,3,38);	     
			OLED_ShowCHinese(48,3,39);	     	     
			OLED_ShowCHinese(64,3,40);	
			break;
		case  0x09:
			OLED_ShowCHinese(0,3,15);	     //没有搜索到指纹
			OLED_ShowCHinese(16,3,16);	  
			OLED_ShowCHinese(32,3,41);	
			OLED_ShowCHinese(48,3,42);	
			OLED_ShowCHinese(64,3,43);	
			OLED_ShowCHinese(80,3,21);	   
			OLED_ShowCHinese(96,3,22);	    
			break;
		case  0x0a:
			OLED_ShowCHinese(0,3,34);	 //  特征合并失败  
			OLED_ShowCHinese(16,3,35);	
			OLED_ShowCHinese(32,3,44);	     
			OLED_ShowCHinese(48,3,45);
			OLED_ShowCHinese(64,3,25);	     
			OLED_ShowCHinese(80,3,26);	 
			break;
		case  0x0b:
			OLED_ShowCHinese(0,3,46);	   //按顺序：地址序号超出范围  
			OLED_ShowCHinese(16,3,47);	    
			OLED_ShowCHinese(32,3,48);	     
			OLED_ShowCHinese(48,3,49);	     	     
			OLED_ShowCHinese(64,3,50);	     
			OLED_ShowCHinese(80,3,51);	
			OLED_ShowCHinese(96,3,52);
			OLED_ShowCHinese(112,3,53);
			break;
		case  0x10:
			OLED_ShowCHinese(16,3,54);	   //按顺序：删除模板失败  
			OLED_ShowCHinese(32,3,55);	    
			OLED_ShowCHinese(48,3,56);	     
			OLED_ShowCHinese(64,3,57);	     	     
			OLED_ShowCHinese(80,3,25);	     
			OLED_ShowCHinese(96,3,26);
			break;
		case  0x11:
			OLED_ShowCHinese(0,3,58);	     
			OLED_ShowCHinese(16,3,59);
			OLED_ShowCHinese(32,3,21);	   //按顺序：清空指纹库失败 
			OLED_ShowCHinese(48,3,22);
			OLED_ShowCHinese(64,3,60);
			OLED_ShowCHinese(80,3,25);	     
			OLED_ShowCHinese(96,3,26);
			break;
		case  0x15:
			OLED_ShowCHinese(0,3,61);	     
			OLED_ShowCHinese(16,3,62);
			OLED_ShowCHinese(32,3,63);	   //按顺序：缓冲区内无有效图 
			OLED_ShowCHinese(48,3,64);
			OLED_ShowCHinese(64,3,65);
			OLED_ShowCHinese(80,3,16);	     
			OLED_ShowCHinese(96,3,66);
			OLED_ShowCHinese(112,3,23);
			break;
		case  0x18:
			OLED_ShowCHinese(0,3,67);	     //读写FLASH出错
			OLED_ShowCHinese(16,3,68);
			OLED_ShowString(32, 3, "FLASH",16);
			OLED_ShowCHinese(72,3,51);
			OLED_ShowCHinese(88,3,9);	
			break;
		case  0x19:
			OLED_ShowCHinese(16,3,69);
			OLED_ShowCHinese(32,3,70);	   //按顺序：未定义错误
			OLED_ShowCHinese(48,3,71);
			OLED_ShowCHinese(64,3,9);
			OLED_ShowCHinese(80,3,10);	  
			break;
		case  0x1a:
			OLED_ShowCHinese(16,3,65);
			OLED_ShowCHinese(32,3,66);	   //按顺序：无效寄存器号
			OLED_ShowCHinese(48,3,72);
			OLED_ShowCHinese(64,3,73);
			OLED_ShowCHinese(80,3,74);	 
			OLED_ShowCHinese(96,3,75);
			break;
		case  0x1b:
			OLED_ShowCHinese(0,3,72);	     
			OLED_ShowCHinese(16,3,73);
			OLED_ShowCHinese(32,3,74);	   //按顺序：寄存器内容错误 
			OLED_ShowCHinese(48,3,64);
			OLED_ShowCHinese(64,3,76);
			OLED_ShowCHinese(80,3,9);	     
			OLED_ShowCHinese(96,3,10);
			break;
		case  0x1c:
			OLED_ShowCHinese(0,3,77);	     
			OLED_ShowCHinese(16,3,78);
			OLED_ShowCHinese(32,3,79);	   //按顺序：记事本页码错误 
			OLED_ShowCHinese(48,3,80);
			OLED_ShowCHinese(64,3,81);
			OLED_ShowCHinese(80,3,9);	     
			OLED_ShowCHinese(96,3,10);
			break;
		case  0x1f:
			OLED_ShowCHinese(32,3,21);	   //按顺序：指纹库满 
			OLED_ShowCHinese(48,3,22);
			OLED_ShowCHinese(64,3,60);
			OLED_ShowCHinese(80,3,82);	 
			break;
		case  0x20:
			OLED_ShowCHinese(32,3,46);	   //按顺序：地址错误 
			OLED_ShowCHinese(48,3,47);
			OLED_ShowCHinese(64,3,9);
			OLED_ShowCHinese(80,3,10);	 
			break;
		default :     
			OLED_ShowCHinese(8,3,83);
			OLED_ShowCHinese(24,3,84);	   //返回确认码有误 
			OLED_ShowCHinese(40,3,85);
			OLED_ShowCHinese(56,3,86);
			OLED_ShowCHinese(72,3,81);
			OLED_ShowCHinese(88,3,16);	     
			OLED_ShowCHinese(104,3,10);
			break;
  }
  return;
}

//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
  EnsureMessage(ensure);
}


//录指纹  可以做个超时检测
void Add_FR(void)
{
  u8 ensure=1;
  u8 ID_NUM = 0;
	u8 processnum = 0;
	OLED_Clear();
  while(1)
  {
		
    switch (processnum)
    {
			case 0:  //第一步
				OLED_ShowCHinese(32,3,99);	   //按顺序：请按指纹 
				OLED_ShowCHinese(48,3,112);
				OLED_ShowCHinese(64,3,21);
				OLED_ShowCHinese(80,3,22);	 
				ensure = PS_GetImage();//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer1); //生成特征
					if(ensure == 0x00)
					{
						OLED_Clear();  //清屏
						OLED_ShowCHinese(32,3,21);	   //按顺序：指纹正常 
						OLED_ShowCHinese(48,3,22);
						OLED_ShowCHinese(64,3,113);
						OLED_ShowCHinese(80,3,114);
						delay_ms(500); //不能超过798
						processnum = 1; //跳到第二步
					}
					else
					{
						OLED_Clear();  //清屏
						ShowErrMessage(ensure);//显示确认码错误信息
						//生成特征失败，重新探测指纹
						processnum = 0;
						delay_ms(500); //不能超过798
						OLED_Clear();  //清屏
					}
						
				}
				else
				{
					OLED_Clear();  //清屏
					//没有探测到手指，继续探测
					processnum = 0;
					ShowErrMessage(ensure);//显示确认码错误信息
					delay_ms(500); //不能超过798
					OLED_Clear();  //清屏
				}
				break;

			case 1:
				OLED_Clear();  //清屏
				OLED_ShowCHinese(24,3,99);	   //按顺序：请再按一次
				OLED_ShowCHinese(40,3,115);
				OLED_ShowCHinese(56,3,112);
				OLED_ShowCHinese(72,3,116);
				OLED_ShowCHinese(88,3,117);
				delay_ms(500); //不能超过798
				ensure = PS_GetImage();//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer2); //生成特征
					if(ensure == 0x00)
					{
						OLED_Clear();  //清屏
						OLED_ShowCHinese(32,3,21);	   //按顺序：指纹正常 
						OLED_ShowCHinese(48,3,22);
						OLED_ShowCHinese(64,3,113);
						OLED_ShowCHinese(80,3,114);
						delay_ms(500); //不能超过798
						OLED_Clear();  //清屏
						processnum = 2; //跳到第三步
					}
					else
					{
						OLED_Clear();  //清屏
						ShowErrMessage(ensure);//显示确认码错误信息
						//生成特征失败，重新探测指纹
						processnum = 0;
						delay_ms(500); //不能超过798
						OLED_Clear();  //清屏
					}
						
				}
				else
				{
					//没有探测到手指，继续探测
					processnum = 1; //继续回到第二部
					OLED_Clear();  //清屏
					ShowErrMessage(ensure);//显示确认码错误信息
					delay_ms(500); //不能超过798
					OLED_Clear();  //清屏
				}
				break;

			case 2:
				OLED_Clear();  //清屏
				OLED_ShowCHinese(16,3,118);	   //按顺序：对比两次指纹 
				OLED_ShowCHinese(32,3,119);
				OLED_ShowCHinese(48,3,120);
				OLED_ShowCHinese(64,3,117);
				OLED_ShowCHinese(80,3,21);
				OLED_ShowCHinese(96,3,11);
				delay_ms(500); //不能超过798
				//精确比对两枚指纹特征 PS_Match
				//功能:精确比对CharBuffer1 与CharBuffer2 中的特征文件
				ensure = PS_Match();
				if(ensure == 0x00)
				{
					OLED_Clear();  //清屏
					OLED_ShowCHinese(32,3,118);	   //按顺序：对比成功 
					OLED_ShowCHinese(48,3,119);
					OLED_ShowCHinese(64,3,95);
					OLED_ShowCHinese(80,3,96);
					delay_ms(500); //不能超过798
					processnum = 3; //跳到第四步
				}
				else
				{
					OLED_Clear();  //清屏
					OLED_ShowCHinese(32,3,118);	   //按顺序：对比失败 
					OLED_ShowCHinese(48,3,119);
					OLED_ShowCHinese(64,3,25);
					OLED_ShowCHinese(80,3,26);
					delay_ms(500); //不能超过798
					OLED_Clear();  //清屏
					ShowErrMessage(ensure);
					delay_ms(500); //不能超过798
					OLED_Clear();  //清屏
					processnum = 0; //跳回第一步
				}
				break;

			case 3:
				OLED_Clear();  //清屏
				OLED_ShowCHinese(16,3,121);	   //按顺序：生成指纹模板 
				OLED_ShowCHinese(32,3,95);	    	     	     
				OLED_ShowCHinese(48,3,21);	     
				OLED_ShowCHinese(64,3,22);
				OLED_ShowCHinese(80,3,56);	     
				OLED_ShowCHinese(96,3,57);
				delay_ms(500); //不能超过798
				//合并特征（生成模板）PS_RegModel
				//功能:将CharBuffer1与CharBuffer2中的特征文件合并生成 模板,结果存于CharBuffer1与CharBuffer2
				ensure = PS_RegModel();
				if(ensure == 0x00)
				{
					OLED_Clear();  //清屏
					OLED_ShowCHinese(0,3,121);	   //按顺序：生成指纹模板成功 
					OLED_ShowCHinese(16,3,95);	    	     	     
					OLED_ShowCHinese(32,3,21);	     
					OLED_ShowCHinese(48,3,22);
					OLED_ShowCHinese(64,3,56);	     
					OLED_ShowCHinese(80,3,57);
					OLED_ShowCHinese(96,3,95);
					OLED_ShowCHinese(112,3,96);
					delay_ms(500); //不能超过798
					processnum = 4; //跳到第五步
				}
				else
				{
					//回到第一步
					OLED_Clear();  //清屏
					processnum = 0;
					ShowErrMessage(ensure);
					delay_ms(500);
					OLED_Clear();  //清屏
				}
				break;

			case 4:
				OLED_Clear();  //清屏
				OLED_ShowString(16,2,"A+B-*",16);
				OLED_ShowCHinese(56,2,122); //保存
				OLED_ShowCHinese(88,2,123);
				OLED_ShowString(0, 4, "  0=< ID <=99   ",16);
				while(1)
				{
					get_key(); //获取按键状态
					if(KeyState !=' ' && KeyState == 'B')
					{
						if(ID_NUM > 0)
						ID_NUM--;
						// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
						KeyState = ' ';
					}
					else if(KeyState !=' ' && KeyState == 'A')
					{
						if(ID_NUM < 99)
						ID_NUM++;
						// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
						KeyState = ' ';
					}
					else if(KeyState !=' ' && KeyState == '*')
					{
						// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
						KeyState = ' ';
						break; //跳出循环存储指纹模板
					}
					OLED_ShowString(16, 6, "YOU ID =",16);
					OLED_ShowNum(80, 6, ID_NUM, 2,16);
				}
				ensure = PS_StoreChar(CharBuffer2, ID_NUM); //储存模板
				if(ensure == 0x00)
				{
					OLED_Clear();
					OLED_ShowCHinese(16,3,19);	   //按顺序：录入指纹成功 
					OLED_ShowCHinese(32,3,20);	    	     	     
					OLED_ShowCHinese(48,3,21);	     
					OLED_ShowCHinese(64,3,22);
					OLED_ShowCHinese(80,3,95);	     
					OLED_ShowCHinese(96,3,96);
					delay_ms(500);
					return ;
				}
				else
				{
					OLED_Clear();
					processnum = 0; //返回第一步
					ShowErrMessage(ensure);
					delay_ms(500);
					OLED_Clear();
				}
				break;
		}
  }
}


SysPara AS608Para;//指纹模块AS608参数
//刷指纹
void press_FR(void)
{
  SearchResult seach;
  u8 ensure;
  char str[20];
	ensure = PS_GetImage();
	if(ensure == 0x00) //获取图像成功
	{
		ensure = PS_GenChar(CharBuffer1);
		if(ensure == 0x00) //生成特征成功
		{
			ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
			if(ensure == 0x00) //搜索成功
			{
				OLED_Clear();
				OLED_ShowCHinese(16,2,21);	   //按顺序：指纹验证成功 
				OLED_ShowCHinese(32,2,22);	    	     	     
				OLED_ShowCHinese(48,2,124);	     
				OLED_ShowCHinese(64,2,125);
				OLED_ShowCHinese(80,2,95);	     
				OLED_ShowCHinese(96,2,96);
				sprintf(str, "ID:%d score:%d" ,seach.pageID, seach.mathscore);
				OLED_ShowString(0, 4, (u8*)str,16);
				delay_ms(500);
				delay_ms(500);
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
			else
			{
				OLED_Clear();
				OLED_ShowCHinese(16,3,21);	   //按顺序：指纹验证失败 
				OLED_ShowCHinese(32,3,22);	    	     	     
				OLED_ShowCHinese(48,3,124);	     
				OLED_ShowCHinese(64,3,125);
				OLED_ShowCHinese(80,3,25);	     
				OLED_ShowCHinese(96,3,26);
				
				PwdAlarmLimit++;
				if(PwdAlarmLimit==8) //错误密码连续超过8次就报警
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
				OLED_Clear();
				OLED_ShowCHinese(0,3,0);	       //小
				OLED_ShowCHinese(18,3,1);	       //亮
				OLED_DrawBMP(46,1,81,7,BMP1);    //显示logo图片
				OLED_ShowCHinese(92,3,2);	       //门
				OLED_ShowCHinese(110,3,3);	     //锁

			}
		}
	}
}

//删除指纹
void Del_FR(void)
{
  u8  ensure;
  u16 ID_NUM = 0;
	OLED_Clear(); 	//清屏
	OLED_ShowString(16,0,"A+B-*",16);
	OLED_ShowCHinese(56,0,97); //确认
	OLED_ShowCHinese(88,0,98);
	OLED_ShowString(16,2, "#",16);
	OLED_ShowCHinese(24,2,110); //清空
	OLED_ShowCHinese(40,2,111);
	OLED_ShowCHinese(56,2,21);	//指纹库
	OLED_ShowCHinese(72,2,22);
	OLED_ShowCHinese(88,2,60);
	OLED_ShowString(16,4, "D",16);
	OLED_ShowCHinese(32,4,126); //退出
	OLED_ShowCHinese(48,4,127);
	int clearflag=1;
  while(1)
  {
		get_key(); //获取按键状态
		if(KeyState !=' ' && KeyState == 'B')
		{
			if(clearflag)
			{
				OLED_Clear();
				clearflag=0;
			}
			if(ID_NUM > 0)
        ID_NUM--;
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}
		else if(KeyState !=' ' && KeyState == 'A')
		{
			if(clearflag)
			{
				OLED_Clear();
				clearflag=0;
			}
			if(ID_NUM < 99)
        ID_NUM++;
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}
		else if(KeyState !=' ' && KeyState == 'D')
		{
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
			break;
			
		}
		else if(KeyState !=' ' && KeyState == '#')
		{
			clearflag=0;
			ensure = PS_Empty(); //清空指纹库
      if(ensure == 0)
      {
        OLED_Clear();
				OLED_ShowCHinese(0,3,58);	     
				OLED_ShowCHinese(16,3,59);
				OLED_ShowCHinese(32,3,21);	   //按顺序：清空指纹库成功 
				OLED_ShowCHinese(48,3,22);
				OLED_ShowCHinese(64,3,60);
				OLED_ShowCHinese(80,3,95);	     
				OLED_ShowCHinese(96,3,96);
				for(int i=0;i<2;i++)
				{
					delay_ms(500);
				}
				OLED_Clear();
      }
      else
			{
				OLED_Clear();
				ShowErrMessage(ensure);
				delay_ms(500);
				OLED_Clear();
			}
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';	
		}
    else if(KeyState !=' ' && KeyState == 'C')
		{
			clearflag=0;
			ensure = PS_DeletChar(ID_NUM, 1); //删除单个指纹
			if(ensure == 0)
			{
				OLED_Clear();
				OLED_ShowCHinese(16,3,54);	     
				OLED_ShowCHinese(32,3,55);
				OLED_ShowCHinese(48,3,21);	   //按顺序：删除指纹成功 
				OLED_ShowCHinese(64,3,22);
				OLED_ShowCHinese(80,3,95);	     
				OLED_ShowCHinese(96,3,96);
				delay_ms(500);
				OLED_Clear();
			}
			else
			{
				OLED_Clear();
				ShowErrMessage(ensure);
				delay_ms(500);
				OLED_Clear();
			}
			// 处理按键完毕后将按键状态重置为' '，避免重复处理同一个按键事件
			KeyState = ' ';
		}
		if(clearflag==0)
		{
			OLED_ShowString(32, 3, "ID = ",16);
			OLED_ShowNum(72, 3, ID_NUM, 2, 16);
		}
    
  }
  
}
