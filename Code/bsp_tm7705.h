#ifndef _BSP_TM7705_H
#define _BSP_TM7705_H

#include "main.h"

void bsp_InitTM7705(void);
void TM7705_CalibSelf(uint8_t _ch);
uint16_t TM7705_ReadAdc(uint8_t _ch);

#endif
