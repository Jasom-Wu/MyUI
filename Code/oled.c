#include "oled.h"
#include "i2c.h"
#include "base_timer.h" 
#include "codetab.h"


idata char DisplayBuffer[BUFFERSIZE];
int OLED_SEND_Cmd(uint8_t cmd)//写操作不同设备不一样,所以要自己定义
{
	i2c_Start();
	i2c_SendByte(OLED_ADDRESS);
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_SendByte(0x00);//对于OLCD,0x00是指令的基地址，写入的指令执行一次，下一次写入时就覆盖了
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_SendByte(cmd);
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_Stop();
	return 1;
	fail:
	i2c_Stop();
	return 0;
}
int OLED_SEND_Data(uint8_t _data)//写操作不同设备不一样,所以要自己定义
{
	i2c_Start();
	i2c_SendByte(OLED_ADDRESS);
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_SendByte(0x40);//对于OLCD,0x40是数据域的基地址
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_SendByte(_data);
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_Stop();
	return 1;
	fail:
	i2c_Stop();
	return 0;
}
void oledInit(void)
{
	delay_ms(500);		// 1s,这里的延时很重要,上电后延时，没有错误的冗余设计
	OLED_SEND_Cmd(0xAE);  //关闭显示
	OLED_SEND_Cmd(0xD5);  //设置时钟分频因子,震荡频率
	OLED_SEND_Cmd(0x80);    //[3:0],分频因子;[7:4],震荡频率
	OLED_SEND_Cmd(0xA8);  //设置驱动路数
	OLED_SEND_Cmd(0X3F);  //默认0X3F(1/64) 
	OLED_SEND_Cmd(0xD3);  //设置显示偏移
	//OLED_SEND_Cmd(0X00);  //默认为0
	OLED_SEND_Cmd(0x00); //设置显示位置—列低地址
	OLED_SEND_Cmd(0x10); //设置显示位置—列高地址
	OLED_SEND_Cmd(0x40);  //设置显示开始行 [5:0],行数.
	OLED_SEND_Cmd(0x8D);  //电荷泵设置
	OLED_SEND_Cmd(0x14);  //bit2，开启/关闭
	OLED_SEND_Cmd(0x20);  //设置内存地址模式
	OLED_SEND_Cmd(0x02);  //[1:0],00，列地址模式; 01，行地址模式; 10,页地址模式;默认10;
	OLED_SEND_Cmd(0xA1);  //段重定义设置,bit0: 0,0->0; 1,0->127;
	//OLED_SEND_Cmd(0xC0);  //设置COM扫描方向;bit3: 0,普通模式; 1,重定义模式 COM[N-1]->COM0; N:驱动路数
	OLED_SEND_Cmd(0xC8);  //设置COM扫描方向
	OLED_SEND_Cmd(0xDA);  //设置COM硬件引脚配置
	OLED_SEND_Cmd(0x12);  //[5:4]配置 
	OLED_SEND_Cmd(0x81);  //对比度设置
	OLED_SEND_Cmd(0xEF);  //1~255;默认0X7F (亮度设置,越大越亮)
	OLED_SEND_Cmd(0xD9);  //设置预充电周期
	OLED_SEND_Cmd(0xf1);  //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_SEND_Cmd(0xDB);  //设置VCOMH 电压倍率
	OLED_SEND_Cmd(0x30);  //[6:4] 000,0.65*vcc; 001,0.77*vcc; 011,0.83*vcc;
	OLED_SEND_Cmd(0xA4);  //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_SEND_Cmd(0xA6);  //设置显示方式;bit0:1,反相显示;0,正常显示              
	OLED_SEND_Cmd(0xAF);  //开启显示 
	OLED_Fill(0x00,0,7); //清屏函数
}

/**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
	*					ch[] :- 要显示的字符串; 
	*					TextSize : 字符大小(1:6*8 ; 2:8*16)
  */
int OLED_ShowStr(unsigned char x, unsigned char y, char *formatString,...)
{
	int c = 0,i = 0,j = 0,length;
	va_list args;
	va_start(args,formatString);
	length = vsprintf(DisplayBuffer,formatString,args);
	while(DisplayBuffer[j] != '\0')
	{
		c = DisplayBuffer[j] - 32;
		if(x > 126)
		{
			x = 0;
			y++;
		}
		OLED_SetPos(x,y);
		for(i=0;i<6;i++)
			OLED_SEND_Data(F6x8[c][i]);
		x += 6;
		j++;
	}
	va_end(args);
	return length;
}
/**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
	*					x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
  */
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
      if(BMP==NULL)
        OLED_SEND_Data(0x00);
      else
			  OLED_SEND_Data(BMP[j++]);
		}
	}
}


void OLED_Fill(unsigned char fill_Data,unsigned char begin,unsigned char end)//按行填充   fill_Data:要填充的数据；begin:起始行，最小为0；end:结束行，最大为7；
{
	unsigned char n,m;
	for(m=begin;m<=end;m++)
	{
		OLED_SEND_Cmd(0xb0+m);		//page0-page1
		OLED_SEND_Cmd(0x00);		//low column start address
		OLED_SEND_Cmd(0x10);		//high column start address
		for(n=0;n<128;n++)
		{
			OLED_SEND_Data(fill_Data);
		}
	}
}
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	OLED_SEND_Cmd(0xb0+y);
	OLED_SEND_Cmd(((x&0xf0)>>4)|0x10);
	OLED_SEND_Cmd((x&0x0f)|0x01);
}
