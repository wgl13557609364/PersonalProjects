//////////////////////////////////////////////////////////////////////////////////	 
//
//  ÎÄ ¼þ Ãû   : main.c
//  °æ ±¾ ºÅ   : 
//  ×÷    Õß   : 
//  Éú³ÉÈÕÆÚ   : 
//  ×î½üÐÞ¸Ä   : 
//  ¹¦ÄÜÃèÊö   : 0.96´çOLED ½Ó¿ÚÑÝÊ¾Àý³Ì(STM32F407ZGÏµÁÐIIC)
//              ËµÃ÷: 
//              ----------------------------------------------------------------
//              GND   µçÔ´µØ
//              VCC   ½Ó5V»ò3.3vµçÔ´
//              SCL   ½ÓPE7£¨SCL£©
//              SDA   ½ÓPE8£¨SDA£©            
//              ----------------------------------------------------------------
//All rights reserved
/////////////////////////////////////////////////////////////////////////////////£

#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"


//OLEDµÄÏÔ´æ
//´æ·Å¸ñÊ½ÈçÏÂ.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   
/**********************************************
//IIC Start
**********************************************/

void Delay2_us(u16 count)
{
  u16 i;
  
  for(i=0;i<count;i++)
  {
		u8 us=12;
		
		while(us--);
  }
}

//SDAÉèÖÃÎªÊä³öÄ£Ê½
static void IIC_SDASetOutputMode(void)
{
	 GPIO_InitTypeDef GPIO_InitStructure;
		
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	 GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			 			//Êä³öÄ£Ê½
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 			//Êä³öËÙ¶È
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		 			  //ÍÆÍì¸´ÓÃ
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;			 			//ÉÏÀ­µç×è
	
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;							//Òý½Å±àºÅ
   GPIO_Init(GPIOE, &GPIO_InitStructure);

}

//SDAÉèÖÃÎªÊäÈëÄ£Ê½
static void IIC_SDASetInputMode(void)
{
	 GPIO_InitTypeDef GPIO_InitStructure;
		
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	 GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;			 			//Êä³öÄ£Ê½
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;			 			//ÉÏÀ­µç×è
	
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;							//Òý½Å±àºÅ
   GPIO_Init(GPIOE, &GPIO_InitStructure);

}

void IIC_Start(void)
{
	//1.SDAÒý½ÅÉèÖÃÎªÊä³öÄ£Ê½
	IIC_SDASetOutputMode();
	
	//2.È·±£SDAºÍSCLÎª¿ÕÏÐ×´Ì¬  ¸ßµçÆ½
	IIC_SDA(1);
	IIC_SCL(1);
	Delay2_us(5);;
	
	//3.ÔÚSCL¸ßµçÆ½ÆÚ¼ä£¬°ÑSDAµçÆ½À­µÍ
	IIC_SDA(0);
	Delay2_us(5);;
	
	//4.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­µÍ
	IIC_SCL(0);
	Delay2_us(5);;

}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop(void)
{
	//1.SDAÒý½ÅÉèÖÃÎªÊä³öÄ£Ê½
	IIC_SDASetOutputMode();
	
	//2.È·±£SDAºÍSCL¶¼Êä³öµÍµçÆ½
	IIC_SDA(0);
	IIC_SCL(0);
	Delay2_us(5);;
	
	//3.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­¸ß
	IIC_SCL(1);
	Delay2_us(5);;
	
	//4.°ÑSDAÊ±ÖÓÏßµÄµçÆ½À­¸ß
	IIC_SDA(1);
	Delay2_us(5);;
	
}

bool IIC_Wait_Ack(void)
{
	bool status; //´æ´¢Ó¦´ð×´Ì¬
	
	//1.ÉèÖÃSDAÊý¾ÝÏßÎªÊäÈëÄ£Ê½
	IIC_SDASetInputMode();
	
	//2.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­µÍ£¬´ËÊ±´Ó»ú¿ÉÒÔ×¼±¸
	IIC_SCL(0);
	Delay2_us(5);;
	
	//3.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­¸ß£¬´ËÊ±Ö÷»ú¿ÉÒÔ¶ÁÈ¡
	IIC_SCL(1);
	Delay2_us(5);;
	
	//4.Ö÷Æ÷¼þ¶ÁÈ¡SDAÊý¾ÝÏßµÄµçÆ½×´Ì¬ µÍµçÆ½±íÊ¾Ó¦´ð
	if( IIC_READ )
		status = false;
	else
		status = true;
	
	Delay2_us(5);;
	
	//5.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­µÍ
	IIC_SCL(0);
	Delay2_us(5);;
	
	return status;
}
/**********************************************
// IIC Write byte
**********************************************/

void Write_IIC_Byte(unsigned char data)
{
	uint8_t i = 0;
	
	//1.SDAÒý½ÅÉèÖÃÎªÊä³öÄ£Ê½
	IIC_SDASetOutputMode();

	//2.°ÑSDAÊý¾ÝÏßºÍSCLÊ±ÖÓÏßµçÆ½À­µÍ
	IIC_SDA(0);
	IIC_SCL(0);
	Delay2_us(5);;
	
	//3.Ñ­»··¢ËÍ8bitÊý¾Ý
	for(i=0;i<8;i++)
	{
		//4.·ÖÎö´ý·¢ËÍÊý¾ÝµÄ×î¸ßÎ» MSB     ¸ßÎ»ÏÈ³ö  
		if( data & (1<<(7-i)) )
			IIC_SDA(1);
		else
			IIC_SDA(0);

		Delay2_us(5);;
		
		//5.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­¸ß£¬´ËÊ±´Ó»ú¿ÉÒÔ¶ÁÈ¡
		IIC_SCL(1);
		Delay2_us(5);;
		
		//6.°ÑSCLÊ±ÖÓÏßµÄµçÆ½À­µÍ£¬´ËÊ±Ö÷»ú¿ÉÒÔ×¼±¸
		IIC_SCL(0);
		Delay2_us(5);;
	}
}
/**********************************************
// IIC Write Command
**********************************************/
void Write_IIC_Command(unsigned char IIC_Command)
{
   IIC_Start();
   Write_IIC_Byte(0x78);            //Slave address,SA0=0
	IIC_Wait_Ack();	
   Write_IIC_Byte(0x00);			//write command
	IIC_Wait_Ack();	
   Write_IIC_Byte(IIC_Command); 
	IIC_Wait_Ack();	
   IIC_Stop();
}
/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
   IIC_Start();
   Write_IIC_Byte(0x78);			//D/C#=0; R/W#=0
	IIC_Wait_Ack();	
   Write_IIC_Byte(0x40);			//write data
	IIC_Wait_Ack();	
   Write_IIC_Byte(IIC_Data);
	IIC_Wait_Ack();	
   IIC_Stop();
}
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
	if(cmd)
			{

   Write_IIC_Data(dat);
   
		}
	else {
   Write_IIC_Command(dat);
		
	}


}


/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char fill_Data)
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		OLED_WR_Byte(0xb0+m,0);		//page0-page1
		OLED_WR_Byte(0x00,0);		//low column start address
		OLED_WR_Byte(0x10,0);		//high column start address
		for(n=0;n<128;n++)
			{
				OLED_WR_Byte(fill_Data,1);
			}
	}
}


/***********************Delay****************************************/
void Delay_50ms(unsigned int Del_50ms)
{
	unsigned int m;
	for(;Del_50ms>0;Del_50ms--)
		for(m=6245;m>0;m--);
}

void Delay_1ms(unsigned int Del_1ms)
{
	unsigned char j;
	while(Del_1ms--)
	{	
		for(j=0;j<123;j++);
	}
}

//×ø±êÉèÖÃ

	void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f),OLED_CMD); 
}   	  
//¿ªÆôOLEDÏÔÊ¾    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDCÃüÁî
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//¹Ø±ÕOLEDÏÔÊ¾     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDCÃüÁî
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//ÇåÆÁº¯Êý,ÇåÍêÆÁ,Õû¸öÆÁÄ»ÊÇºÚÉ«µÄ!ºÍÃ»µãÁÁÒ»Ñù!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //ÉèÖÃÒ³µØÖ·£¨0~7£©
		OLED_WR_Byte (0x00,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐµÍµØÖ·
		OLED_WR_Byte (0x10,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐ¸ßµØÖ·   
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} //¸üÐÂÏÔÊ¾
}
void OLED_On(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //ÉèÖÃÒ³µØÖ·£¨0~7£©
		OLED_WR_Byte (0x00,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐµÍµØÖ·
		OLED_WR_Byte (0x10,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐ¸ßµØÖ·   
		for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA); 
	} //¸üÐÂÏÔÊ¾
}
//ÔÚÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö×Ö·û,°üÀ¨²¿·Ö×Ö·û
//x:0~127
//y:0~63
//mode:0,·´°×ÏÔÊ¾;1,Õý³£ÏÔÊ¾				 
//size:Ñ¡Ôñ×ÖÌå 16/12 
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';//µÃµ½Æ«ÒÆºóµÄÖµ			
		if(x>Max_Column-1){x=0;y=y+2;}
		if(Char_Size ==16)
			{
			OLED_Set_Pos(x,y);	
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
			}
			else {	
				OLED_Set_Pos(x,y);
				for(i=0;i<6;i++)
				OLED_WR_Byte(F6x8[c][i],OLED_DATA);
				
			}
}
//m^nº¯Êý
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//ÏÔÊ¾2¸öÊý×Ö
//x,y :Æðµã×ø±ê	 
//len :Êý×ÖµÄÎ»Êý
//size:×ÖÌå´óÐ¡
//mode:Ä£Ê½	0,Ìî³äÄ£Ê½;1,µþ¼ÓÄ£Ê½
//num:ÊýÖµ(0~4294967295);	 		  
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size2)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2); 
	}
} 
//ÏÔÊ¾Ò»¸ö×Ö·ûºÅ´®
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j],Char_Size);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}
//ÏÔÊ¾ºº×Ö
void OLED_ShowCHinese(u8 x,u8 y,u8 no)
{      			    
	u8 t,adder=0;
	OLED_Set_Pos(x,y);	
    for(t=0;t<16;t++)
		{
				OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
				adder+=1;
     }	
		OLED_Set_Pos(x,y+1);	
    for(t=0;t<16;t++)
			{	
				OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
				adder+=1;
      }					
}
/***********¹¦ÄÜÃèÊö£ºÏÔÊ¾ÏÔÊ¾BMPÍ¼Æ¬128¡Á64ÆðÊ¼µã×ø±ê(x,y),xµÄ·¶Î§0¡«127£¬yÎªÒ³µÄ·¶Î§0¡«7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
} 


//³õÊ¼»¯SSD1306					    
void OLED_Init(void)
{ 	
 
 	//Òý½ÅÅäÖÃ  SDA--PE8  SCL--PE7
	GPIO_InitTypeDef GPIO_InitStructure;
		
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			 		//Êä³öÄ£Ê½
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 			//Êä³öËÙ¶È
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		 			//ÍÆÍì¸´ÓÃ
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;			 			//ÉÏÀ­µç×è

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;							//Òý½Å±àºÅ
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;							//Òý½Å±àºÅ
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
 
	volatile unsigned int num; //us
  volatile unsigned int t;
	for(int i;i<300;i++) //ÑÓ³Ù200ms
	{
		for (num = 0; num < 1000; num++)
		{
			t = 11;
			while (t != 0)
			{
				t--;
			}
		}
	}

	

	OLED_WR_Byte(0xAE,OLED_CMD);//--display off
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  
	OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
	OLED_WR_Byte(0x81,OLED_CMD); // contract control
	OLED_WR_Byte(0xFF,OLED_CMD);//--128   
	OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap 
	OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty
	OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
	OLED_WR_Byte(0x00,OLED_CMD);//
	
	OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
	OLED_WR_Byte(0x80,OLED_CMD);//
	
	OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
	OLED_WR_Byte(0x05,OLED_CMD);//
	
	OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
	OLED_WR_Byte(0xF1,OLED_CMD);//
	
	OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
	OLED_WR_Byte(0x12,OLED_CMD);//
	
	OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//
	
	OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
	OLED_WR_Byte(0x14,OLED_CMD);//
	
	OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
}  





























