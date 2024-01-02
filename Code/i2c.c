#include "i2c.h"

sbit BUS_SCL = P2^0;
sbit BUS_SDA = P2^1;

#define i2c_Delay() ;//����51��˵���Բ��ӳ�

void i2c_Start()
{
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C���������ź� */
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
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C����ֹͣ�ź� */
	I2C_SDA=0;
	I2C_SCL=1;
	i2c_Delay();
	I2C_SDA=1;
}

void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* �ȷ����ֽڵĸ�λbit7 */
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
			 I2C_SDA=1; // �ͷ�����
		}
		_ucByte <<= 1;	/* ����һ��bit */
		i2c_Delay();
	}
}


uint8_t i2c_WaitAck()//���ʹ�������Ӧ��������0
{
	uint8_t re;

	I2C_SDA=1;	/* CPU�ͷ�SDA���� */
	i2c_Delay();
	I2C_SCL=1;	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
	i2c_Delay();
	if (I2C_SDA)	/* CPU��ȡSDA����״̬ */
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
