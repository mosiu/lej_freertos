/**
  ******************************************************************************
  * @file    Templates_LL/Src/main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    14-April-2017
  * @brief   Main program body through the LL API
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "led.h"
#include "USART.h"
//#include "DMA.h"

#define RED_LED_PIN_MASK                    LL_GPIO_PIN_10
#define RED_LED_GPIO_PORT                  	GPIOB
#define GREEN_LED_PIN_MASK					LL_GPIO_PIN_2
#define GREEN_LED_GPIO_PORT					GPIOB


/** @addtogroup STM32F1xx_LL_Examples
  * @{
  */

/** @addtogroup Templates_LL
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void RTOS_tasks_create(void);

uint8_t flag = 0;
TaskHandle_t Handle_red_led;
TaskHandle_t Handle_green_led;
TaskHandle_t Handle_USART;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Configure the system clock to 24 MHz */
  SystemClock_Config();

  /* Add your application code here */
  // important: makes sure that there are no subpriority bits
  // reference: Cortex-M3 TRM, table 5-3
  NVIC_SetPriorityGrouping(0);


  RTOS_tasks_create();

  vTaskStartScheduler();



  /* Infinite loop */
  while (1)
  {

  }
}

//void vApplicationIdleHook( void )
//{
//	while(1)
//	{
//	  int i=0;
//	  if (i)
//		  i++;
//	  i--;
//	}
//}

void
RTOS_tasks_create(void)
{
	static LEDTaskParams red_led_params, green_led_params;
	red_led_params.LedGPIO = RED_LED_GPIO_PORT;
	red_led_params.LedPinMask = RED_LED_PIN_MASK;
	red_led_params.tick_interval_ms = 500;
	green_led_params.LedGPIO = GREEN_LED_GPIO_PORT;
	green_led_params.LedPinMask = GREEN_LED_PIN_MASK;
	green_led_params.tick_interval_ms = 200;

	xTaskCreate(LED_Task,	// function with infinite loop that realizes the task
				"RedLED",	// name of the task for debug tracing
				100,		// stack depth
				(void *)&red_led_params,	// parameters to pass
				(UBaseType_t)3,	// priority (lowest possible is 5)
				&Handle_red_led);	// handle to reference the task later on

	configASSERT(Handle_red_led);	// assert if task was not created (the handle would be null)

	xTaskCreate(LED_Task,	// function with infinite loop that realizes the task
				"GreenLED",	// name of the task for debug tracing
				100,		// stack depth
				(void *)&green_led_params,	// parameters to pass
				(UBaseType_t)5,	// priority (lowest possible is 5)
				&Handle_green_led);	// handle to reference the task later on

	configASSERT(Handle_green_led);

	xTaskCreate(USART_task,
				"USART",
				100,
				NULL,
				(UBaseType_t)4,
				NULL);

	Handle_USART = xTaskGetHandle("USART");
	configASSERT(Handle_USART);
}
/* ==============   BOARD SPECIFIC CONFIGURATION CODE BEGIN    ============== */
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 24000000
  *            HCLK(Hz)                       = 24000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 2
  *            PLLMUL                         = 6
  *            Flash Latency(WS)              = 0
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  /* Enable HSI oscillator - 8MHz */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  };

  /* Main PLL configuration and activation - HSI/2 as PLL source, then multiplied by 6 */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_6);
  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };

  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };

  /* Set APB1 & APB2 prescaler */
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms */
  SysTick_Config(24000000 / 1000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  SystemCoreClock = 24000000;
}


/* ==============   BOARD SPECIFIC CONFIGURATION CODE END      ============== */

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
