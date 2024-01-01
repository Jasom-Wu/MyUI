#include "main.h"
#include "base_timer.h"
#include "bsp_uart.h"
#include "bsp_tm7705.h"
#include "bsp_task.h"
#include "my_ui.h"


void main(){
	
	bsp_InitUart();
	baseTimerInit();
	MenuInit();
	bsp_InitTM7705();			/* ³õÊ¼»¯ÅäÖÃTM7705 */
	
	while(1){
//		taskShowRMS();
    taskMenuCore();
		
//		float volt = getRMS(128);
//		uint16_t adc = TM7705_ReadAdc(2);
//		volt = (float)adc*1000/(13005*0.985);
//		OLED_ShowStr(2,1,2,"%6.1fmV",volt);
//		OLED_ShowStr(2,3,2,"ADC:%6d",adc);
//		if(getTick()-tick>=1000)break;
	}
//	OLED_Fill(0,1,4);
//	OLED_ShowStr(2,1,2,"FPS:%d",adc_count);
	
}	