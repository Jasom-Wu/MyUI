#include "bsp_tm7705.h"

/* 通道1和通道2的增益,输入缓冲，极性 */
#define __CH1_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)
#define __CH2_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)


	/* 定义GPIO端口 */
	sbit CS     = P0^0;
	sbit RESET  = P0^1;
	sbit DIN    = P0^2;
	sbit SCK    = P0^3;
	sbit DOUT   = P0^4;
	sbit DRDY	= P0^5;

	/* 定义口线置0和置1的宏 */
	#define CS_0()		CS = 0
	#define CS_1()		CS = 1

	#define RESET_0()	RESET = 0
	#define RESET_1()	RESET = 1

	#define DI_0()		DIN = 0
	#define DI_1()		DIN = 1

	#define SCK_0()		SCK = 0 
	#define SCK_1()		SCK = 1

	#define DO_IS_HIGH()	(DOUT != 0)

	#define DRDY_IS_LOW()	(DRDY == 0)


/* 通信寄存器bit定义 */
enum
{
	/* 寄存器选择  RS2 RS1 RS0  */
	REG_COMM	= 0x00,				/* 通信寄存器 */
	REG_SETUP	= 0x10,				/* 设置寄存器 */
	REG_CLOCK	= 0x20,				/* 时钟寄存器 */
	REG_DATA	= 0x30,				/* 数据寄存器 */
	REG_ZERO_CH1	= 0x60,		/* CH1 偏移寄存器 */
	REG_FULL_CH1	= 0x70,		/* CH1 满量程寄存器 */
	REG_ZERO_CH2	= 0x61,		/* CH2 偏移寄存器 */
	REG_FULL_CH2	= 0x71,		/* CH2 满量程寄存器 */

	/* 读写操作 */
	WRITE 		= 0x00,				/* 写操作 */
	READ 		= 0x08,					/* 读操作 */

	/* 通道 */
	CH_1		= 0,	/* AIN1+  AIN1- */
	CH_2		= 1,	/* AIN2+  AIN2- */
	CH_3		= 2,	/* AIN1-  AIN1- */
	CH_4		= 3		/* AIN1-  AIN2- */
};

/* 设置寄存器bit定义 */
enum
{
	MD_NORMAL		= (0 << 6),		/* 正常模式 */
	MD_CAL_SELF		= (1 << 6),	/* 自校准模式 */
	MD_CAL_ZERO		= (2 << 6),	/* 校准0刻度模式 */
	MD_CAL_FULL		= (3 << 6),	/* 校准满刻度模式 */

	GAIN_1			= (0 << 3),	/* 增益 */
	GAIN_2			= (1 << 3),	/* 增益 */
	GAIN_4			= (2 << 3),	/* 增益 */
	GAIN_8			= (3 << 3),	/* 增益 */
	GAIN_16			= (4 << 3),	/* 增益 */
	GAIN_32			= (5 << 3),	/* 增益 */
	GAIN_64			= (6 << 3),	/* 增益 */
	GAIN_128		= (7 << 3),	/* 增益 */

	/* 无论双极性还是单极性都不改变任何输入信号的状态，它只改变输出数据的代码和转换函数上的校准点 */
	BIPOLAR			= (0 << 2),	/* 双极性输入 */
	UNIPOLAR		= (1 << 2),	/* 单极性输入 */

	BUF_NO			= (0 << 1),	/* 输入无缓冲（内部缓冲器不启用) */
	BUF_EN			= (1 << 1),	/* 输入有缓冲 (启用内部缓冲器) */

	FSYNC_0			= 0,
	FSYNC_1			= 1					/* 不启用 */
};

/* 时钟寄存器bit定义 */
enum
{
	CLKDIS_0	= 0x00,		/* 时钟输出使能 （当外接晶振时，必须使能才能振荡） */
	CLKDIS_1	= 0x10,		/* 时钟禁止 （当外部提供时钟时，设置该位可以禁止MCK_OUT引脚输出时钟以省电 */

	/*
		2.4576MHz（CLKDIV=0 ）或为 4.9152MHz （CLKDIV=1 ），CLK 应置 “0”。
		1MHz （CLKDIV=0 ）或 2MHz   （CLKDIV=1 ），CLK 该位应置  “1”
	*/
	CLK_4_9152M = 0x08,
	CLK_2_4576M = 0x00,

	FS_50HZ		= 0x00,
	FS_60HZ		= 0x01,
	FS_250HZ	= 0x02,
	FS_500HZ	= 0x03,//不是0x04

	ZERO_0		= 0x00,
	ZERO_1		= 0x80
};

static void TM7705_SyncSPI(void);
static void TM7705_Send8Bit(uint8_t _data);
static uint8_t TM7705_Recive8Bit(void);
static void TM7705_WriteByte(uint8_t _data);
static uint16_t TM7705_Read2Byte(void);
static void TM7705_WaitDRDY(void);
static void TM7705_ResetHard(void);
static void bsp_DelayMS(uint16_t n);

#define TM7705_Delay() ;
/*
*	函 数 名: bsp_InitTM7705
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 TM7705
*	形    参: 无
*	返 回 值: 无
*/
void bsp_InitTM7705(void)
{
	bsp_DelayMS(10);

	TM7705_ResetHard();		/* 硬件复位 */

	/*
		在接口序列丢失的情况下，如果在DIN 高电平的写操作持续了足够长的时间（至少 32个串行时钟周期），
		TM7705 将会回到默认状态。
	*/
	bsp_DelayMS(5);

	TM7705_SyncSPI();		/* 同步SPI接口时序 */

	bsp_DelayMS(5);

	/* 配置时钟寄存器 */
	TM7705_WriteByte(REG_CLOCK | WRITE | CH_2);			/* 先写通信寄存器，下一步是写时钟寄存器 */

	TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_500HZ);	/* 刷新速率50Hz */
	//TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_500HZ);	/* 刷新速率500Hz */

	/* 每次上电进行一次自校准 */
	TM7705_CalibSelf(2);	/* 内部自校准 CH1 */
	bsp_DelayMS(5);
}
static void bsp_DelayMS(uint16_t n)
{
	/* 
	   以下代码，已经通过逻辑分析验证。	bsp_DelayMS(500) 时，实际延迟498ms 
	*/
	uint16_t k;

	while(n--)
	{
		for (k = 0; k < 75; k++);
	}
}
/*
*	函 数 名: TM7705_ResetHard
*	功能说明: 硬件复位 TM7705芯片
*	形    参: 无
*	返 回 值: 无
*/
static void TM7705_ResetHard(void)
{
	RESET_1();
	bsp_DelayMS(1);
	RESET_0();
	bsp_DelayMS(1);
	RESET_1();
	bsp_DelayMS(1);
}

/*
*	函 数 名: TM7705_SyncSPI
*	功能说明: 同步TM7705芯片SPI接口时序
*	形    参: 无
*	返 回 值: 无
*/
static void TM7705_SyncSPI(void)
{
	/* AD7705串行接口失步后将其复位。复位后要延时500us再访问 */
	CS_0();
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	CS_1();
}

/*
*	函 数 名: TM7705_Send8Bit
*	功能说明: 向SPI总线发送8个bit数据。 不带CS控制。
*	形    参: _data : 数据
*	返 回 值: 无
*/
static void TM7705_Send8Bit(uint8_t _data)
{
	uint8_t i;

	for(i = 0; i < 8; i++)
	{
		if (_data & 0x80)
		{
			DI_1();
		}
		else
		{
			DI_0();
		}
		SCK_0();
		_data <<= 1;
		TM7705_Delay();
		SCK_1();
		TM7705_Delay();
	}
}

/*
*	函 数 名: TM7705_Recive8Bit
*	功能说明: 从SPI总线接收8个bit数据。 不带CS控制。
*	形    参: 无
*	返 回 值: 无
*/
static uint8_t TM7705_Recive8Bit(void)
{
	uint8_t i;
	uint8_t read = 0;

	for (i = 0; i < 8; i++)
	{
		SCK_0();
		TM7705_Delay();
		read = read<<1;
		if (DO_IS_HIGH())
		{
			read++;
		}
		SCK_1();
		TM7705_Delay();
	}
	return read;
}

/*
*	函 数 名: TM7705_WriteByte
*	功能说明: 写入1个字节。带CS控制
*	形    参: _data ：将要写入的数据
*	返 回 值: 无
*/
static void TM7705_WriteByte(uint8_t _data)
{
	CS_0();
	TM7705_Send8Bit(_data);
	CS_1();
}

/*
*	函 数 名: TM7705_Read2Byte
*	功能说明: 读2字节数据
*	形    参: 无
*	返 回 值: 读取的数据（16位）
*/
static uint16_t TM7705_Read2Byte(void)
{
	uint16_t read;

	CS_0();
	read = TM7705_Recive8Bit();
	read <<= 8;
	read += TM7705_Recive8Bit();
	CS_1();

	return read;
}


/*
*	函 数 名: TM7705_WaitDRDY
*	功能说明: 等待内部操作完成。 自校准时间较长，需要等待。
*	形    参: 无
*	返 回 值: 无
*/
static void TM7705_WaitDRDY(void)
{
	uint16_t i;

	for (i = 0; i < 15000; i++)
	{
		if (DRDY_IS_LOW())
		{
			break;
		}
	}
	if (i >= 15000)
	{
//		printf("TM7705_WaitDRDY() 芯片应答超时 ...\r\n");		/* 调试语句. 用语排错 */

//		printf("重新同步SPI接口时序\r\n");		/* 调试语句. 用语排错 */

		TM7705_SyncSPI();		/* 同步SPI接口时序 */

		bsp_DelayMS(5);
	}
}

/*
*	函 数 名: TM7705_CalibSelf
*	功能说明: 启动自校准. 内部自动短接AIN+ AIN-校准0位，内部短接到Vref 校准满位。此函数执行过程较长，
*			  实测约 180ms
*	形    参:  _ch : ADC通道，1或2
*	返 回 值: 无
*/
void TM7705_CalibSelf(uint8_t _ch)
{
	if (_ch == 1)
	{
		/* 自校准CH1 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);	/* 写通信寄存器，下一步是写设置寄存器，通道1 */
		TM7705_WriteByte(MD_CAL_SELF | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成 --- 时间较长，约180ms */
	}
	else if (_ch == 2)
	{
		/* 自校准CH2 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);	/* 写通信寄存器，下一步是写设置寄存器，通道2 */
		TM7705_WriteByte(MD_CAL_SELF | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);	/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成  --- 时间较长，约180ms */
	}
}


/*
*	函 数 名: TM7705_ReadAdc1
*	功能说明: 读通道1或2的ADC数据
*	形    参: 无
*	返 回 值: 无
*/
uint16_t TM7705_ReadAdc(uint8_t _ch)
{
	uint8_t i;
	uint16_t read = 0;

	/* 为了避免通道切换造成读数失效，读2次 */
	for (i = 0; i < 1; i++)
	{
		TM7705_WaitDRDY();		/* 等待DRDY口线为0 */

		if (_ch == 1)
		{
			TM7705_WriteByte(0x38);
		}
		else if (_ch == 2)
		{
			TM7705_WriteByte(0x39);
		}

		read = TM7705_Read2Byte();
	}
	return read;
}


float getRMS(uint16_t len){
  float volt_sum=0;
  float RMS=0,volt;
  uint16_t adc,i;
  i=len;
  if(len>0){
    while(i){
      adc = TM7705_ReadAdc(2);
      if(adc==3583){bsp_InitTM7705();continue;}//概率性出现固定错误adc值，需要复位
      volt = (float)adc*1000/(13000*0.974);
      volt_sum += volt*volt;
      i--;
    }
    RMS = sqrt((float)(volt_sum/len));//单位换算以及校准
    RMS -=1645;//减去偏置电压值
    RMS *= 5;//乘上电压增益
  }
  return RMS;
}

