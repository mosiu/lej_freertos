/*
 * USART.h
 *
 *  Created on: 24.12.2017
 *      Author: Mosiu
 */

#ifndef EXAMPLE_USER_USART_H_
#define EXAMPLE_USER_USART_H_

typedef enum {
	USART_SUCCESS = 0,
	USART_QUEUE_FULL,
	USART_NULLPTR
} USARTstatus;

void USART_task(void *params);
void USART_print_task_list(void);
USARTstatus USART_print_message(const char *str);

#endif /* EXAMPLE_USER_USART_H_ */
