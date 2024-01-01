#include "bsp_task.h"

sbit Buzzer=P2^5;

//code uint16_t beep_tunes[8]={200,182,160,147,130,112,100,93};
//code uint16_t beep_delays[10]={50,60,70,80,90,100,110,120,130,140};

//sbit LED = P2^2;
////===========声音处理区=============
//void playNote(uint16_t _tune,uint16_t _delay)
//{//默认占空比为50%
//  uint16_t x,t=_tune;
//  for(x=_delay;x>0;x--)//持续_delay时间
//  {
//    Buzzer=~Buzzer;//取反振荡
//    while(t--);//决定了振荡频率
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