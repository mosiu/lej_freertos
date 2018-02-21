/*
 * led.c
 *
 *  Created on: 20.12.2017
 *      Author: Mosiu
 */

#include "led.h"
#include "main.h"
#include "USART.h"

void LED_Init(LEDTaskParams params);
void LED_Toggle(LEDTaskParams params);


void LED_Task(void *params)
{
	LEDTaskParams LED_Params;
	LED_Params = *((LEDTaskParams *)params);

	LED_Init(LED_Params);

	int toggle_count = 0;
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(LED_Params.tick_interval_ms));
		LED_Toggle(LED_Params);

		toggle_count ++;
		if (toggle_count == 20)
		{
			if (xTaskGetCurrentTaskHandle() == Handle_green_led)
			{
				USART_print_message("Green\n");
			}
			else
			{
				USART_print_message("\tRed\n");

			}
			toggle_count = 0;
		}
	}
}

void LED_Init(LEDTaskParams params)
{
	// enable clock
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

	LL_GPIO_SetPinMode(params.LedGPIO, params.LedPinMask, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetOutputPin(params.LedGPIO, params.LedPinMask);

}

void LED_Toggle(LEDTaskParams params)
{
	LL_GPIO_TogglePin(params.LedGPIO, params.LedPinMask);
}
