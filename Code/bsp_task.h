#ifndef _BSP_TASK_H_
#define _BSP_TASK_H_

#include "main.h"
#include "bsp_tm7705.h"
#include "base_timer.h"
#include "oled.h"
#include "my_ui.h"

#define TASK_MENU_CORE_MS_PER_TICK 100

void taskMenuCore(void);



#endif