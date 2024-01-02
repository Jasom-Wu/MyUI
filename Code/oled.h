#ifndef __OLED_H_
#define __OLED_H_

#include "stdarg.h"
#include "stdio.h"
#include "main.h"


#define OLED_ADDRESS	0x78   //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78
#define BUFFERSIZE 50

void oledInit(void);
void OLED_CLS(uint8_t mode);
int OLED_ShowStr(unsigned char x, unsigned char y, char *formatString,...);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
int OLED_SEND_Data(uint8_t _data);
int OLED_SEND_Cmd(uint8_t cmd);
void OLED_Fill(unsigned char fill_Data,unsigned char begin,unsigned char end);
void OLED_SetPos(unsigned char x, unsigned char y);
#endif//__OLED_H_
