#ifndef _BSP_TASK_H_
#define _BSP_TASK_H_

#include "main.h"
#include "bsp_tm7705.h"
#include "math.h"
#include "base_timer.h"
#include "oled.h"
#include "my_ui.h"

#define MS_PER_TICK 	10 

void taskMenuCore(void);
void taskShowRMS(void);
float getRMS(uint16_t len);


#endif