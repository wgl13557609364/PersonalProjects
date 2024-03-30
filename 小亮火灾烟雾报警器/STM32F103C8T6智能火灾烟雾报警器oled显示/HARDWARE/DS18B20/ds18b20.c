/************************************************************************************

//  功能描述   : DS18B20模块测试程序
//  测试条件   : STM32F103C8T6   晶振8M  系统时钟72M
接线
DS18B20--------------------------------STM32F103C8T6
VCC------------------------------------5V
GND------------------------------------GND
OUT------------------------------------PA10


OLED0.96-------------------------------STM32F103C8T6
VCC------------------------------------3.3V
GND------------------------------------GND
SCL -----------------------------------PB4 
SDA------------------------------------PB5;

*************************************************************************************/
#include "ds18b20.h"
#include "delay.h"
//uchar data_byte; 
//uchar RH,RL,TH,TL; 
unsigned char  DisplayData[15];
int D_temp=0;
//DS18B20配置为输出模式，引脚是GPIOA_10
void  DS18B20_OUT_GPIO_Config(void)
{
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			DS18B20_DAT_1;
}
//DelayUs配置为上拉输入模式，引脚是GPIOA_10
void  DS18B20_IN_GPIO_Config(void)
{
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}
uchar  Ds18b20Init(void)//开始信号 
{ 
	uchar i;
	DS18B20_OUT_GPIO_Config();
	DS18B20_DAT_0;		 	 //将总线拉低480us~960us
	Delay_us(650);
	DS18B20_DAT_1;			//然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低
	DS18B20_IN_GPIO_Config();
	while(DS18B20_DAT_Read)	//等待DS18B20拉低总线
	{
		Delay_ms(1);
		i++;
		if(i>5)//等待>5MS
		{
			return 0;//初始化失败
		}
	
	}
	return 1;//初始化成功

	

} 

/*******************************************************************************
* 函 数 名         : Ds18b20WriteByte
* 函数功能		   : 向18B20写入一个字节
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void Ds18b20WriteByte(uchar dat)
{
	uint  j;
	DS18B20_OUT_GPIO_Config();
	for(j=0; j<8; j++)
	{
		DS18B20_DAT_0;	     	  //每写入一位数据之前先把总线拉低1us
		Delay_us(2);
		if(dat & 0x01)
		{
			DS18B20_DAT_1 ;  //然后写入一个数据，从最低位开始
			
		}
		else
		{
			DS18B20_DAT_0;
			
		}
		Delay_us(68);
		DS18B20_DAT_1;	//然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
		Delay_us(2);
		dat >>= 1;
	}
}
/*******************************************************************************
* 函 数 名         : Ds18b20ReadByte
* 函数功能		   : 读取一个字节
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/


uchar Ds18b20ReadByte(void)
{
	uchar byte, bi;
	uint  j;	
	
	for(j=8; j>0; j--)
	{
		DS18B20_OUT_GPIO_Config();
		DS18B20_DAT_0;//先将总线拉低1us
		Delay_us(2);
		DS18B20_DAT_1;//然后释放总线
		Delay_us(6);
		DS18B20_IN_GPIO_Config();
		bi = DS18B20_DAT_Read;	 //读取数据，从最低位开始读取
		/*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
		byte = (byte >> 1) | (bi << 7);						  
		Delay_us(48);		//读取完之后等待48us再接着读取下一个数
		
	}				
	return byte;
}
/*******************************************************************************
* 函 数 名         : Ds18b20ChangTemp
* 函数功能		   : 让18b20开始转换温度
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void  Ds18b20ChangTemp(void)
{
	Ds18b20Init();
	Delay_ms(1);
	Ds18b20WriteByte(0xcc);		//跳过ROM操作命令		 
	Ds18b20WriteByte(0x44);	    //温度转换命令
	//Delay1ms(100);	//等待转换成功，而如果你是一直刷着的话，就不用这个延时了
   
}
/*******************************************************************************
* 函 数 名         : Ds18b20ReadTempCom
* 函数功能		   : 发送读取温度命令
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void  Ds18b20ReadTempCom(void)
{	

	Ds18b20Init();
	Delay_ms(1);
	Ds18b20WriteByte(0xcc);	 //跳过ROM操作命令
	Ds18b20WriteByte(0xbe);	 //发送读取温度命令
}
/*******************************************************************************
* 函 数 名         : Ds18b20ReadTemp
* 函数功能		   : 读取温度
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

int Ds18b20ReadTemp(void)
{
	int temp = 0;
	uchar tmh, tml;
	Ds18b20ChangTemp();			 	//先写入转换命令
	Ds18b20ReadTempCom();			//然后等待转换完后发送读取温度命令
	tml = Ds18b20ReadByte();		//读取温度值共16位，先读低字节
	tmh = Ds18b20ReadByte();		//再读高字节
	temp = tmh;
	temp <<= 8;
	temp |= tml;
	return temp;
}
void datapros(int temp) 	 
{
   	float tp;  
	if(temp< 0)				//当温度值为负数
  	{
		DisplayData[0] = 0x40; 	  //   -
		//因为读取的温度是实际温度的补码，所以减1，再取反求出原码
		temp=temp-1;
		temp=~temp;
		tp=temp;
		temp=tp*0.0625*100+0.5;	
		//留两个小数点就*100，+0.5是四舍五入，因为C语言浮点数转换为整型的时候把小数点
		//后面的数自动去掉，不管是否大于0.5，而+0.5之后大于0.5的就是进1了，小于0.5的就
		//算加上0.5，还是在小数点后面。
 
  	}
 	else
  	{			
		DisplayData[0] = 0x00;
		tp=temp;//因为数据处理有小数点所以将温度赋给一个浮点型变量
		//如果温度是正的那么，那么正数的原码就是补码它本身
		temp=tp*0.0625*100+0.5;	
		//留两个小数点就*100，+0.5是四舍五入，因为C语言浮点数转换为整型的时候把小数点
		//后面的数自动去掉，不管是否大于0.5，而+0.5之后大于0.5的就是进1了，小于0.5的就
		//算加上0.5，还是在小数点后面。
	}
	D_temp=temp;

}





