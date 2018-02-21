/*
 * USART.c
 *
 *  Created on: 24.12.2017
 *      Author: Mosiu
 */

#include "main.h"
#include "USART.h"
#include "queue.h"
#include "portmacro.h"


#define CHANNEL DMA1_Channel7

#define QUEUE_LENGTH			3
#define STRING_LENGTH			0x20	// short 31-character strings

typedef uint32_t StringLength_t;
typedef char Char_t;
typedef struct {
	StringLength_t length;
	char string[STRING_LENGTH];
} TxQueueItemType;

static const UBaseType_t ItemSize = sizeof(TxQueueItemType);

static QueueHandle_t TxQueue = NULL;
#define INIT_MSG_LENGTH 24
static Char_t * InitMsg = "USART: DMA initialized!\n";

static void DMA_Init2(void);
static void DMA_Reload(TxQueueItemType * item);
static void USART_Init(void);

void USART_task(void *params)
{
	USART_Init();

	TxQueue = xQueueCreate(QUEUE_LENGTH, ItemSize);

	Char_t item_memory[ItemSize];
	while(1)
	{
//		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		void * item_ptr = item_memory;
		xQueueReceive(TxQueue, item_ptr, portMAX_DELAY);
		DMA_Reload((TxQueueItemType *)item_ptr);
#ifdef DEBUG
		vTaskSuspend(Handle_USART);
#else
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
#endif
	}
}

#define VALUE_THAT_HAS_NO_MEANING_BECAUSE_THE_QUEUE_IS_NOT_FULL	6

USARTstatus USART_print_message(const char *str)
{
	if (str == NULL || TxQueue == NULL)
	{
		return USART_NULLPTR;
	}

	if (xQueueIsQueueFullFromISR(TxQueue))
	{
		return USART_QUEUE_FULL;
	}

	// allocate memory
	Char_t new_item[ItemSize];	// space for length and string
	StringLength_t i;
	const Char_t * string_ptr = str;

	// copy string to blocks 4-N of a new queue item
	for (i=0;
			(*string_ptr != '\0') && (i < STRING_LENGTH);
			string_ptr++, i++)
	{
		new_item[i + sizeof(StringLength_t)] = *string_ptr;
	}

	// save string length in block 0 of a new queue item
	*((StringLength_t *)new_item) = i;	// masterpiece: cast tab address to StringLength_t* and then use * operator to make it StringLength_t variable
	// TODO:: CHECK FOR ENDIANNESS ISSUES

	configASSERT(xQueueSendToBack(TxQueue, (void*) new_item, VALUE_THAT_HAS_NO_MEANING_BECAUSE_THE_QUEUE_IS_NOT_FULL));

#ifdef DEBUG
	UBaseType_t queue_elements;
	queue_elements = uxQueueMessagesWaiting(TxQueue);
#endif
	return USART_SUCCESS;
}

void USART2_IRQHandler(void)
{
	if ((USART2->SR & USART_SR_TC))
	{
		USART2->SR &= ~USART_SR_TC;	// needs to be cleared explicitly, for we don't write to DR register explicitly - DMA does it.

		static uint32_t entry_count = 1;
		if (entry_count == 1)
		{
			// enable DMA channel
			CHANNEL->CCR |= DMA_CCR_EN;
		}
		if (entry_count >= 3)
		{
			xTaskResumeFromISR(Handle_USART);
		}
		entry_count ++;
	}
}

void DMA_Reload(TxQueueItemType * item)
{
	// disable channel
	CHANNEL->CCR &= ~DMA_CCR_EN;
	// set shit again
	CHANNEL->CNDTR = item->length;
	CHANNEL->CMAR = &item->string;
	// enable DMA channel
	CHANNEL->CCR |= DMA_CCR_EN;
}

void USART_Init(void)
{
	// enable AHB bus clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	DMA_Init2();

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_2, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_2, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_2, LL_GPIO_PULL_UP);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_3, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_3, LL_GPIO_PULL_UP);

	LL_USART_InitTypeDef init_struct;
	LL_USART_StructInit(&init_struct);

	USART2->CR3 |= USART_CR3_DMAT;
	configASSERT(LL_USART_Init(USART2, &init_struct));

	USART2->CR1 |= USART_CR1_TCIE;	// when I enter an interrupt from USART 2, I can be sure that TC is set. Then I can clear it and enable DMA channel.
									// TC will occur whenever DMA finished. I will implement further instructions for USART in this ISR.

	LL_USART_Enable(USART2);
	LL_USART_EnableDirectionTx(USART2);

	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 6);


}

void DMA_Init2(void)
{
	//DMA periph config. P - as source, M - as dest

	//set address
	CHANNEL->CPAR = (uint32_t)&(USART2->DR);
	CHANNEL->CMAR = (uint32_t)InitMsg;
	CHANNEL->CNDTR = INIT_MSG_LENGTH;
	// 				high prio 	| 8-bit alignment for memory and peripheral | transfer complete interrupt | increment mode for memory | direction: read from memory
	CHANNEL->CCR |= (DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_DIR);
	CHANNEL->CCR &= ~ (DMA_CCR_MSIZE_0 | DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_0 | DMA_CCR_PSIZE_1 | DMA_CCR_PINC);	// these bits are non-zero by default

}

