/*
 * MYRTOS_ERROR_STATE.h
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

#ifndef INC_MYRTOS_ERROR_STATE_H_
#define INC_MYRTOS_ERROR_STATE_H_

typedef enum
{
	ES_NoError,
	ES_Ready_Queue_Init_Error,
	ES_Error_Task_Exceeded_Stack_Size
}MYRTOS_ES_t;

#endif /* INC_MYRTOS_ERROR_STATE_H_ */
