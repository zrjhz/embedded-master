#include "stm32f4xx.h"
#include "sys.h"

#ifndef __INFRARED_H
#define __INFRARED_H

void Infrared_Init(void);
void Infrared_Send(uint8_t *s, int n);

#endif
