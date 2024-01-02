#include "bsp_task.h"


void taskMenuCore(void){
  static uint32_t last_tick = 0;
  if (getTick() - last_tick > TASK_MENU_CORE_MS_PER_TICK) {
    MenuKeyHandler();
    MenuProcessHandler();
    last_tick = getTick();
  }
}

