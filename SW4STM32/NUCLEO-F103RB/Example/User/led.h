/*
 * led.h
 *
 *  Created on: 20.12.2017
 *      Author: Mosiu
 */

#ifndef EXAMPLE_USER_LED_H_
#define EXAMPLE_USER_LED_H_

#include "main.h"

typedef struct {
	GPIO_TypeDef * LedGPIO;
	uint32_t LedPinMask;
	uint32_t tick_interval_ms;
} LEDTaskParams;

void LED_Task (void *params);

#endif /* EXAMPLE_USER_LED_H_ */
