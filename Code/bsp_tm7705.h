#ifndef _BSP_TM7705_H
#define _BSP_TM7705_H

#include "main.h"
#include "math.h"

void bsp_InitTM7705(void);
void TM7705_CalibSelf(void);
uint16_t TM7705_ReadAdc(void);
void setGain(uint8_t gain);
uint8_t getGain(void);
float getRMS(uint16_t len,uint8_t gain,float *DC_volt);

#endif
