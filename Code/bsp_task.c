#include "bsp_task.h"



void taskMenuCore(void){
  static idata uint16_t last_tick = 0;
  if (getTick() - last_tick > 200) {
    MenuKeyHandler();
    MenuProcessHandler();



    last_tick = getTick();
  }
}
void taskShowRMS(void){
	static idata uint16_t last_tick=0;
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