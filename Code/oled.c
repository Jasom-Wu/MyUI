#include "oled.h"
#include "i2c.h"
#include "base_timer.h" 
#include "codetab.h"


idata char DisplayBuffer[BUFFERSIZE];
int OLED_SEND_Cmd(uint8_t cmd)//д������ͬ�豸��һ��,����Ҫ�Լ�����
{
	i2c_Start();
	i2c_SendByte(OLED_ADDRESS);
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_SendByte(0x00);//����OLCD,0x00��ָ��Ļ���ַ��д���ָ��ִ��һ�Σ���һ��д��ʱ�͸�����
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
int OLED_SEND_Data(uint8_t _data)//д������ͬ�豸��һ��,����Ҫ�Լ�����
{
	i2c_Start();
	i2c_SendByte(OLED_ADDRESS);
	if(i2c_WaitAck() !=0)
	goto fail;
	i2c_SendByte(0x40);//����OLCD,0x40��������Ļ���ַ
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
	delay_ms(500);		// 1s,�������ʱ����Ҫ,�ϵ����ʱ��û�д�����������
	OLED_SEND_Cmd(0xAE);  //�ر���ʾ
	OLED_SEND_Cmd(0xD5);  //����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_SEND_Cmd(0x80);    //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_SEND_Cmd(0xA8);  //��������·��
	OLED_SEND_Cmd(0X3F);  //Ĭ��0X3F(1/64) 
	OLED_SEND_Cmd(0xD3);  //������ʾƫ��
	//OLED_SEND_Cmd(0X00);  //Ĭ��Ϊ0
	OLED_SEND_Cmd(0x00); //������ʾλ�á��е͵�ַ
	OLED_SEND_Cmd(0x10); //������ʾλ�á��иߵ�ַ
	OLED_SEND_Cmd(0x40);  //������ʾ��ʼ�� [5:0],����.
	OLED_SEND_Cmd(0x8D);  //��ɱ�����
	OLED_SEND_Cmd(0x14);  //bit2������/�ر�
	OLED_SEND_Cmd(0x20);  //�����ڴ��ַģʽ
	OLED_SEND_Cmd(0x02);  //[1:0],00���е�ַģʽ; 01���е�ַģʽ; 10,ҳ��ַģʽ;Ĭ��10;
	OLED_SEND_Cmd(0xA1);  //���ض�������,bit0: 0,0->0; 1,0->127;
	//OLED_SEND_Cmd(0xC0);  //����COMɨ�跽��;bit3: 0,��ͨģʽ; 1,�ض���ģʽ COM[N-1]->COM0; N:����·��
	OLED_SEND_Cmd(0xC8);  //����COMɨ�跽��
	OLED_SEND_Cmd(0xDA);  //����COMӲ����������
	OLED_SEND_Cmd(0x12);  //[5:4]���� 
	OLED_SEND_Cmd(0x81);  //�Աȶ�����
	OLED_SEND_Cmd(0xEF);  //1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_SEND_Cmd(0xD9);  //����Ԥ�������
	OLED_SEND_Cmd(0xf1);  //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_SEND_Cmd(0xDB);  //����VCOMH ��ѹ����
	OLED_SEND_Cmd(0x30);  //[6:4] 000,0.65*vcc; 001,0.77*vcc; 011,0.83*vcc;
	OLED_SEND_Cmd(0xA4);  //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	OLED_SEND_Cmd(0xA6);  //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ              
	OLED_SEND_Cmd(0xAF);  //������ʾ 
	OLED_Fill(0x00,0,7); //��������
}

/**
  * @brief  OLED_ShowStr����ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
  * @param  x,y : ��ʼ������(x:0~127, y:0~7);
	*					ch[] :- Ҫ��ʾ���ַ���; 
	*					TextSize : �ַ���С(1:6*8 ; 2:8*16)
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
  * @brief  OLED_DrawBMP����ʾBMPλͼ
  * @param  x0,y0 :��ʼ������(x0:0~127, y0:0~7);
	*					x1,y1 : ���Խ���(������)������(x1:1~128,y1:1~8)
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


void OLED_Fill(unsigned char fill_Data,unsigned char begin,unsigned char end)//�������   fill_Data:Ҫ�������ݣ�begin:��ʼ�У���СΪ0��end:�����У����Ϊ7��
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
void OLED_SetPos(unsigned char x, unsigned char y) //������ʼ������
{ 
	OLED_SEND_Cmd(0xb0+y);
	OLED_SEND_Cmd(((x&0xf0)>>4)|0x10);
	OLED_SEND_Cmd((x&0x0f)|0x01);
}
