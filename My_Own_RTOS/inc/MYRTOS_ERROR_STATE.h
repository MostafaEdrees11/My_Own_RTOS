/*
 * MYRTOS_ERROR_STATE.h
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

#ifndef INC_MYRTOS_ERROR_STATE_H_
#define INC_MYRTOS_ERROR_STATE_H_

/*
 * Enumeration Name: MYRTOS_ES_t
 * Usage:          : it has all cases of errors that may happen in our rtos
 */
typedef enum
{
	ES_NoError,
	ES_Ready_Queue_Init_Error,
	ES_Error_Task_Exceeded_Stack_Size,
	ES_Error_Bubble_Sort,
	ES_Error_SysTick_counting,
	ES_Error_Many_User_Mutex
}MYRTOS_ES_t;

#endif /* INC_MYRTOS_ERROR_STATE_H_ */
