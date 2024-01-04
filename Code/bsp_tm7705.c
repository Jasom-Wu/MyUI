#include "bsp_tm7705.h"
#include "base_timer.h"

/* ͨ��1��ͨ��2������,���뻺�壬���� */
#define __CH1_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)
#define __CH2_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)


	/* ����GPIO�˿� */
	sbit CS     = P1^1;
	sbit DIN    = P1^2;
	sbit DRDY		= P1^3;
	sbit RESET  = P1^4;
	sbit SCK    = P1^5;
	sbit DOUT   = P1^6;
	

	/* ���������0����1�ĺ� */
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


/* ͨ�żĴ���bit���� */
enum
{
	/* �Ĵ���ѡ��  RS2 RS1 RS0  */
	REG_COMM	= 0x00,				/* ͨ�żĴ��� */
	REG_SETUP	= 0x10,				/* ���üĴ��� */
	REG_CLOCK	= 0x20,				/* ʱ�ӼĴ��� */
	REG_DATA	= 0x30,				/* ���ݼĴ��� */
	REG_ZERO_CH1	= 0x60,		/* CH1 ƫ�ƼĴ��� */
	REG_FULL_CH1	= 0x70,		/* CH1 �����̼Ĵ��� */
	REG_ZERO_CH2	= 0x61,		/* CH2 ƫ�ƼĴ��� */
	REG_FULL_CH2	= 0x71,		/* CH2 �����̼Ĵ��� */

	/* ��д���� */
	WRITE 		= 0x00,				/* д���� */
	READ 		= 0x08,					/* ������ */

	/* ͨ�� */
	CH_1		= 0,	/* AIN1+  AIN1- */
	CH_2		= 1,	/* AIN2+  AIN2- */
	CH_3		= 2,	/* AIN1-  AIN1- */
	CH_4		= 3		/* AIN1-  AIN2- */
};

/* ���üĴ���bit���� */
enum
{
	MD_NORMAL		= (0 << 6),		/* ����ģʽ */
	MD_CAL_SELF		= (1 << 6),	/* ��У׼ģʽ */
	MD_CAL_ZERO		= (2 << 6),	/* У׼0�̶�ģʽ */
	MD_CAL_FULL		= (3 << 6),	/* У׼���̶�ģʽ */

	GAIN_1			= (0 << 3),	/* ���� */
	GAIN_2			= (1 << 3),	/* ���� */
	GAIN_4			= (2 << 3),	/* ���� */
	GAIN_8			= (3 << 3),	/* ���� */
	GAIN_16			= (4 << 3),	/* ���� */
	GAIN_32			= (5 << 3),	/* ���� */
	GAIN_64			= (6 << 3),	/* ���� */
	GAIN_128		= (7 << 3),	/* ���� */

	/* ����˫���Ի��ǵ����Զ����ı��κ������źŵ�״̬����ֻ�ı�������ݵĴ����ת�������ϵ�У׼�� */
	BIPOLAR			= (0 << 2),	/* ˫�������� */
	UNIPOLAR		= (1 << 2),	/* ���������� */

	BUF_NO			= (0 << 1),	/* �����޻��壨�ڲ�������������) */
	BUF_EN			= (1 << 1),	/* �����л��� (�����ڲ�������) */

	FSYNC_0			= 0,
	FSYNC_1			= 1					/* ������ */
};

/* ʱ�ӼĴ���bit���� */
enum
{
	CLKDIS_0	= 0x00,		/* ʱ�����ʹ�� ������Ӿ���ʱ������ʹ�ܲ����񵴣� */
	CLKDIS_1	= 0x10,		/* ʱ�ӽ�ֹ �����ⲿ�ṩʱ��ʱ�����ø�λ���Խ�ֹMCK_OUT�������ʱ����ʡ�� */

	/*
		2.4576MHz��CLKDIV=0 ����Ϊ 4.9152MHz ��CLKDIV=1 ����CLK Ӧ�� ��0����
		1MHz ��CLKDIV=0 ���� 2MHz   ��CLKDIV=1 ����CLK ��λӦ��  ��1��
	*/
	CLK_4_9152M = 0x08,
	CLK_2_4576M = 0x00,

	FS_50HZ		= 0x00,
	FS_60HZ		= 0x01,
	FS_250HZ	= 0x02,
	FS_500HZ	= 0x03,//����0x04

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

#define TM7705_Delay() ;
#define bsp_DelayMS(x) delay_ms(x);
/*
*	�� �� ��: bsp_InitTM7705
*	����˵��: ����STM32��GPIO��SPI�ӿڣ��������� TM7705
*	��    ��: ��
*	�� �� ֵ: ��
*/
void bsp_InitTM7705(void)
{

	TM7705_ResetHard();		/* Ӳ����λ */

	/*
		�ڽӿ����ж�ʧ������£������DIN �ߵ�ƽ��д�����������㹻����ʱ�䣨���� 32������ʱ�����ڣ���
		TM7705 ����ص�Ĭ��״̬��
	*/
	bsp_DelayMS(5);

	TM7705_SyncSPI();		/* ͬ��SPI�ӿ�ʱ�� */

	bsp_DelayMS(5);

	/* ����ʱ�ӼĴ��� */
	TM7705_WriteByte(REG_CLOCK | WRITE | CH_2);			/* ��дͨ�żĴ�������һ����дʱ�ӼĴ��� */

	TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_500HZ);	/* ˢ������50Hz */
	//TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_500HZ);	/* ˢ������500Hz */

	/* ÿ���ϵ����һ����У׼ */
	TM7705_CalibSelf();	/* �ڲ���У׼ CH1 */
	bsp_DelayMS(5);
}

/*
*	�� �� ��: TM7705_ResetHard
*	����˵��: Ӳ����λ TM7705оƬ
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: TM7705_SyncSPI
*	����˵��: ͬ��TM7705оƬSPI�ӿ�ʱ��
*	��    ��: ��
*	�� �� ֵ: ��
*/
static void TM7705_SyncSPI(void)
{
	/* AD7705���нӿ�ʧ�����临λ����λ��Ҫ��ʱ500us�ٷ��� */
	CS_0();
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	CS_1();
}

/*
*	�� �� ��: TM7705_Send8Bit
*	����˵��: ��SPI���߷���8��bit���ݡ� ����CS���ơ�
*	��    ��: _data : ����
*	�� �� ֵ: ��
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
*	�� �� ��: TM7705_Recive8Bit
*	����˵��: ��SPI���߽���8��bit���ݡ� ����CS���ơ�
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: TM7705_WriteByte
*	����˵��: д��1���ֽڡ���CS����
*	��    ��: _data ����Ҫд�������
*	�� �� ֵ: ��
*/
static void TM7705_WriteByte(uint8_t _data)
{
	CS_0();
	TM7705_Send8Bit(_data);
	CS_1();
}

/*
*	�� �� ��: TM7705_Read2Byte
*	����˵��: ��2�ֽ�����
*	��    ��: ��
*	�� �� ֵ: ��ȡ�����ݣ�16λ��
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
*	�� �� ��: TM7705_WaitDRDY
*	����˵��: �ȴ��ڲ�������ɡ� ��У׼ʱ��ϳ�����Ҫ�ȴ���
*	��    ��: ��
*	�� �� ֵ: ��
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
		TM7705_SyncSPI();		/* ͬ��SPI�ӿ�ʱ�� */
		bsp_DelayMS(5);
	}
}

/*
*	�� �� ��: TM7705_CalibSelf
*	����˵��: ������У׼. �ڲ��Զ��̽�AIN+ AIN-У׼0λ���ڲ��̽ӵ�Vref У׼��λ���˺���ִ�й��̽ϳ���
*			  ʵ��Լ 180ms
*	�� �� ֵ: ��
*/
void TM7705_CalibSelf(void){
	/* ��У׼CH2 */
	TM7705_WriteByte(REG_SETUP | WRITE | CH_2);	/* дͨ�żĴ�������һ����д���üĴ�����ͨ��2 */
	TM7705_WriteByte(MD_CAL_SELF | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);	/* ������У׼ */
	TM7705_WaitDRDY();	/* �ȴ��ڲ��������  --- ʱ��ϳ���Լ180ms */
}


/*
*	�� �� ��: TM7705_ReadAdc
*	����˵��: ��ͨ��2��ADC����
*	��    ��: ��
*	�� �� ֵ: ��
*/
uint16_t TM7705_ReadAdc()
{
	TM7705_WaitDRDY();		/* �ȴ�DRDY����Ϊ0 */
	TM7705_WriteByte(0x39);
	return TM7705_Read2Byte();
}

void setGain(uint8_t gain){
  TM7705_WriteByte(0x11);//д���üĴ���,ͨ��2
	TM7705_WriteByte(0x06|(gain<<3));//00xxx110;
	TM7705_WaitDRDY();
}

uint8_t getGain(void){
  uint8_t gain;
  TM7705_WaitDRDY();
  TM7705_WriteByte(0x19);//��ȡ���üĴ���,ͨ��2
  CS_0();
  gain = TM7705_Recive8Bit();
  CS_1();
  gain = (gain>>3) & 0x07;//��ȡgain
  return gain;
}


float getRMS(uint16_t len,uint8_t gain,float *DC_volt){
  float volt_sum=0;
  float RMS=0,DC=0,volt;
  uint16_t i;
	int16_t adc;
  i=len*2;
  if(len>0){
    while(i){
			if(i==len){
				DC = volt_sum/len;
				volt_sum = 0;
			}
      adc = TM7705_ReadAdc();
      if(adc==3583)//�����Գ��̶ֹ�����adcֵ����Ҫ��λ
				{bsp_InitTM7705();continue;}
      volt = ((float)adc*5.0/(1<<gain)/65536)*2;//��λ����
			if(i>len)
				volt_sum += volt;
			else
				volt_sum += (volt-DC)*(volt-DC);
      i--;
    }
    RMS = sqrt((float)(volt_sum/(float)len))*1000;//�����mV
  }
	*DC_volt = DC*1000;
  return RMS;
}

