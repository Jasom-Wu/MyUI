#include "base_timer.h"


static idata uint32_t tick=0;

void baseTimerInit(){
	TMOD &=0XF0;
	TMOD |=0X01;
	EA = 1;
	ET0 = 1;
	TL0 = 0X66;
	TH0 = 0XFC;
	TR0 = 1;
	tick = 0;
}

void systemTickHandler() interrupt 1
{
	TL0 = 0X66;
	TH0 = 0XFC;
	
	tick++;
}
uint32_t getTick(){
	return tick;
}
void delay_ms(uint32_t xms){
	uint32_t end_ms = tick+xms;
	while(tick<end_ms);
}
