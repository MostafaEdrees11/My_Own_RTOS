/*
 * MYRTOS_FIFO.c
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

#include "MYRTOS_FIFO.h"

/**
 **********************************************************************************
 * @function_name	: FIFO_init
 * @arguments		: pointer to LIFO, pointer to Buffer, length of buffer
 * @brief			: it's used to initialize the FIFO to be ready to use it.
 **********************************************************************************
 **/
FIFO_Buf_Status FIFO_init (FIFO_Buf_t *P_fifo, FIFO_element_type *buf, uint32_t length)
{
	//check if buffer is reserved at memory or not
	if(buf == NULL)
		return FIFO_null;

	//initialize fifo
	P_fifo->base = buf;
	P_fifo->tail = P_fifo->base;
	P_fifo->head = P_fifo->base;
	P_fifo->length = length;
	P_fifo->count = 0;

	return FIFO_no_error;
}


/**
 **********************************************************************************
 * @function_name	: FIFO_IS_FULL
 * @arguments		: pointer to LIFO
 * @brief			: it's used to check if fifo is full or not.
 **********************************************************************************
 **/
FIFO_Buf_Status FIFO_IS_FULL(FIFO_Buf_t *P_fifo)
{
	//check if fifo is valid or not.
	if(!P_fifo->base || !P_fifo->tail || !P_fifo->head)
		return FIFO_null;

	if(P_fifo->count == P_fifo->length)
		return FIFO_full;

	return FIFO_no_error;
}

/**
 **********************************************************************************
 * @function_name	: FIFO_IS_EMPTY
 * @arguments		: pointer to LIFO
 * @brief			: it's used to check if fifo is empty or not.
 **********************************************************************************
 **/
FIFO_Buf_Status FIFO_IS_EMPTY(FIFO_Buf_t *P_fifo)
{
	//check if fifo is valid or not.
	if(!P_fifo->base || !P_fifo->tail || !P_fifo->head)
		return FIFO_null;

	if(P_fifo->count == 0)
		return FIFO_empty;

	return FIFO_no_error;
}

/**
 **********************************************************************************
 * @function_name	: FIFO_Enqueue_Item
 * @arguments		: pointer to LIFO, item value to enqueue it.
 * @brief			: it's used to enqueue value to the fifo.
 **********************************************************************************
 **/
FIFO_Buf_Status FIFO_Enqueue_Item(FIFO_Buf_t *P_fifo, FIFO_element_type item)
{
	//check if fifo is valid or not.
	if(!P_fifo->base || !P_fifo->tail || !P_fifo->head)
		return FIFO_null;

	//check if fifo is full or not
	if(FIFO_IS_FULL(P_fifo) == FIFO_full)
		return FIFO_full;

	//enqueue the item to fifo
	*(P_fifo->head) = item;
	P_fifo->count++;

	//check if head at the end of queue or not.
	if(P_fifo->head == (P_fifo->base + (P_fifo->length * sizeof(FIFO_element_type))))
		P_fifo->head = P_fifo->base;
	else
		P_fifo->head++;

	return FIFO_no_error;
}

/**
 **********************************************************************************
 * @function_name	: FIFO_Dequeue_Item
 * @arguments		: pointer to LIFO, pointer to the item dequeued from fifo.
 * @brief			: it's used to dequeue value from the fifo.
 **********************************************************************************
 **/
FIFO_Buf_Status FIFO_Dequeue_Item(FIFO_Buf_t *P_fifo, FIFO_element_type *item)
{
	//check if fifo is valid or not.
	if(!P_fifo->base || !P_fifo->tail || !P_fifo->head)
		return FIFO_null;

	//check if the fifo is empty or not.
	if(FIFO_IS_EMPTY(P_fifo) == FIFO_empty)
		return FIFO_empty;

	//dequeue the item from fifo
	*item = *(P_fifo->tail);
	P_fifo->count--;

	//check if tail at the end of queue or not.
	if(P_fifo->tail == (P_fifo->base + (P_fifo->length * sizeof(FIFO_element_type))))
		P_fifo->tail = P_fifo->base;
	else
		P_fifo->tail++;

	return FIFO_no_error;
}

