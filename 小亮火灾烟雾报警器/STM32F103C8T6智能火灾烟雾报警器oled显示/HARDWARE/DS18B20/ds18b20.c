/************************************************************************************

//  ��������   : DS18B20ģ����Գ���
//  ��������   : STM32F103C8T6   ����8M  ϵͳʱ��72M
����
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
//DS18B20����Ϊ���ģʽ��������GPIOA_10
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
//DelayUs����Ϊ��������ģʽ��������GPIOA_10
void  DS18B20_IN_GPIO_Config(void)
{
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}
uchar  Ds18b20Init(void)//��ʼ�ź� 
{ 
	uchar i;
	DS18B20_OUT_GPIO_Config();
	DS18B20_DAT_0;		 	 //����������480us~960us
	Delay_us(650);
	DS18B20_DAT_1;			//Ȼ���������ߣ����DS18B20������Ӧ�Ὣ��15us~60us����������
	DS18B20_IN_GPIO_Config();
	while(DS18B20_DAT_Read)	//�ȴ�DS18B20��������
	{
		Delay_ms(1);
		i++;
		if(i>5)//�ȴ�>5MS
		{
			return 0;//��ʼ��ʧ��
		}
	
	}
	return 1;//��ʼ���ɹ�

	

} 

/*******************************************************************************
* �� �� ��         : Ds18b20WriteByte
* ��������		   : ��18B20д��һ���ֽ�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

void Ds18b20WriteByte(uchar dat)
{
	uint  j;
	DS18B20_OUT_GPIO_Config();
	for(j=0; j<8; j++)
	{
		DS18B20_DAT_0;	     	  //ÿд��һλ����֮ǰ�Ȱ���������1us
		Delay_us(2);
		if(dat & 0x01)
		{
			DS18B20_DAT_1 ;  //Ȼ��д��һ�����ݣ������λ��ʼ
			
		}
		else
		{
			DS18B20_DAT_0;
			
		}
		Delay_us(68);
		DS18B20_DAT_1;	//Ȼ���ͷ����ߣ�����1us�����߻ָ�ʱ����ܽ���д��ڶ�����ֵ
		Delay_us(2);
		dat >>= 1;
	}
}
/*******************************************************************************
* �� �� ��         : Ds18b20ReadByte
* ��������		   : ��ȡһ���ֽ�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/


uchar Ds18b20ReadByte(void)
{
	uchar byte, bi;
	uint  j;	
	
	for(j=8; j>0; j--)
	{
		DS18B20_OUT_GPIO_Config();
		DS18B20_DAT_0;//�Ƚ���������1us
		Delay_us(2);
		DS18B20_DAT_1;//Ȼ���ͷ�����
		Delay_us(6);
		DS18B20_IN_GPIO_Config();
		bi = DS18B20_DAT_Read;	 //��ȡ���ݣ������λ��ʼ��ȡ
		/*��byte����һλ��Ȼ����������7λ���bi��ע���ƶ�֮���Ƶ���λ��0��*/
		byte = (byte >> 1) | (bi << 7);						  
		Delay_us(48);		//��ȡ��֮��ȴ�48us�ٽ��Ŷ�ȡ��һ����
		
	}				
	return byte;
}
/*******************************************************************************
* �� �� ��         : Ds18b20ChangTemp
* ��������		   : ��18b20��ʼת���¶�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

void  Ds18b20ChangTemp(void)
{
	Ds18b20Init();
	Delay_ms(1);
	Ds18b20WriteByte(0xcc);		//����ROM��������		 
	Ds18b20WriteByte(0x44);	    //�¶�ת������
	//Delay1ms(100);	//�ȴ�ת���ɹ������������һֱˢ�ŵĻ����Ͳ��������ʱ��
   
}
/*******************************************************************************
* �� �� ��         : Ds18b20ReadTempCom
* ��������		   : ���Ͷ�ȡ�¶�����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

void  Ds18b20ReadTempCom(void)
{	

	Ds18b20Init();
	Delay_ms(1);
	Ds18b20WriteByte(0xcc);	 //����ROM��������
	Ds18b20WriteByte(0xbe);	 //���Ͷ�ȡ�¶�����
}
/*******************************************************************************
* �� �� ��         : Ds18b20ReadTemp
* ��������		   : ��ȡ�¶�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

int Ds18b20ReadTemp(void)
{
	int temp = 0;
	uchar tmh, tml;
	Ds18b20ChangTemp();			 	//��д��ת������
	Ds18b20ReadTempCom();			//Ȼ��ȴ�ת������Ͷ�ȡ�¶�����
	tml = Ds18b20ReadByte();		//��ȡ�¶�ֵ��16λ���ȶ����ֽ�
	tmh = Ds18b20ReadByte();		//�ٶ����ֽ�
	temp = tmh;
	temp <<= 8;
	temp |= tml;
	return temp;
}
void datapros(int temp) 	 
{
   	float tp;  
	if(temp< 0)				//���¶�ֵΪ����
  	{
		DisplayData[0] = 0x40; 	  //   -
		//��Ϊ��ȡ���¶���ʵ���¶ȵĲ��룬���Լ�1����ȡ�����ԭ��
		temp=temp-1;
		temp=~temp;
		tp=temp;
		temp=tp*0.0625*100+0.5;	
		//������С�����*100��+0.5���������룬��ΪC���Ը�����ת��Ϊ���͵�ʱ���С����
		//��������Զ�ȥ���������Ƿ����0.5����+0.5֮�����0.5�ľ��ǽ�1�ˣ�С��0.5�ľ�
		//�����0.5��������С������档
 
  	}
 	else
  	{			
		DisplayData[0] = 0x00;
		tp=temp;//��Ϊ���ݴ�����С�������Խ��¶ȸ���һ�������ͱ���
		//����¶���������ô����ô������ԭ����ǲ���������
		temp=tp*0.0625*100+0.5;	
		//������С�����*100��+0.5���������룬��ΪC���Ը�����ת��Ϊ���͵�ʱ���С����
		//��������Զ�ȥ���������Ƿ����0.5����+0.5֮�����0.5�ľ��ǽ�1�ˣ�С��0.5�ľ�
		//�����0.5��������С������档
	}
	D_temp=temp;

}





