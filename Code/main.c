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
	bsp_InitTM7705();
	
	while(1){
    taskMenuCore();
	}
}	