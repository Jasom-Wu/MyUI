#ifndef __I2C_H_
#define __I2C_H_
#include "reg52.h"
#include "main.h"

//=======================================================================================================
#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */
#define I2C_SCL BUS_SCL
#define I2C_SDA BUS_SDA
//=======================================================================================================

//以下为应用函数
void i2c_SendByte(uint8_t _ucByte);
//以下为驱动函数
void i2c_Start();
void i2c_Stop();
uint8_t i2c_WaitAck();

#endif
