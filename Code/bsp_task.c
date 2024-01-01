#include "bsp_task.h"

sbit Buzzer=P2^5;

//code uint16_t beep_tunes[8]={200,182,160,147,130,112,100,93};
//code uint16_t beep_delays[10]={50,60,70,80,90,100,110,120,130,140};

//sbit LED = P2^2;
////===========����������=============
//void playNote(uint16_t _tune,uint16_t _delay)
//{//Ĭ��ռ�ձ�Ϊ50%
//  uint16_t x,t=_tune;
//  for(x=_delay;x>0;x--)//����_delayʱ��
//  {
//    Buzzer=~Buzzer;//ȡ����
//    while(t--);//��������Ƶ��
//    t=_tune;
//  }
//}

void taskMenuCore(void){
  static uint32_t last_tick = 0;
  if (getTick() - last_tick > 200) {
    MenuKeyHandler();
    MenuProcessHandler();
    last_tick = getTick();
  }
}
void taskShowRMS(void){
	static uint32_t last_tick=0;
	float volt;
	if(getTick()-last_tick>500){
		volt = getRMS(256);
		OLED_ShowStr(2,1,2,"%6.1fmV",volt);
		last_tick = getTick();
	}
}

float getRMS(uint16_t len){
	float volt_sum=0;
	float RMS=0,volt;
	uint16_t adc,i;
	i=len;
	if(len>0){
		while(i){
			adc = TM7705_ReadAdc(2);
			if(adc==3583){bsp_InitTM7705();continue;}//�����Գ��̶ֹ�����adcֵ����Ҫ��λ
			volt = (float)adc*1000/(13000*0.974);
			volt_sum += volt*volt;
			i--;
		}
		RMS = sqrt((float)(volt_sum/len));//��λ�����Լ�У׼
		RMS -=1645;//��ȥƫ�õ�ѹֵ
		RMS *= 5;//���ϵ�ѹ����
	}
	return RMS;
}