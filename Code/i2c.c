#include "i2c.h"

sbit BUS_SCL = P2^0;
sbit BUS_SDA = P2^1;

#define i2c_Delay() ;//对于51来说可以不延迟

void i2c_Start()
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	I2C_SDA=1;
	I2C_SCL=1;
	i2c_Delay();
	I2C_SDA=0;
	i2c_Delay();
	I2C_SCL=0;
	i2c_Delay();
}

void i2c_Stop()
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	I2C_SDA=0;
	I2C_SCL=1;
	i2c_Delay();
	I2C_SDA=1;
}

void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++)
	{		
		if (_ucByte & 0x80)
		{
			I2C_SDA=1;
		}
		else
		{
			I2C_SDA=0;
		}
		i2c_Delay();
		I2C_SCL=1;
		i2c_Delay();	
		I2C_SCL=0;
		if (i == 7)
		{
			 I2C_SDA=1; // 释放总线
		}
		_ucByte <<= 1;	/* 左移一个bit */
		i2c_Delay();
	}
}


uint8_t i2c_WaitAck()//拉低代表有响应，即返回0
{
	uint8_t re;

	I2C_SDA=1;	/* CPU释放SDA总线 */
	i2c_Delay();
	I2C_SCL=1;	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	i2c_Delay();
	if (I2C_SDA)	/* CPU读取SDA口线状态 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	I2C_SCL=0;
	i2c_Delay();
	return re;
}
