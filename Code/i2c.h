#ifndef __I2C_H_
#define __I2C_H_
#include "reg52.h"
#include "main.h"

//=======================================================================================================
#define I2C_WR	0		/* д����bit */
#define I2C_RD	1		/* ������bit */
#define I2C_SCL BUS_SCL
#define I2C_SDA BUS_SDA
//=======================================================================================================

//����ΪӦ�ú���
void i2c_SendByte(uint8_t _ucByte);
//����Ϊ��������
void i2c_Start();
void i2c_Stop();
uint8_t i2c_WaitAck();

#endif
