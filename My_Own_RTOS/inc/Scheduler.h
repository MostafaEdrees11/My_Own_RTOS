/*
 * Scheduler.h
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

/*
 * ------------
 * | Includes |
 * ------------
 */
#include "CortexMx_OS_Porting.h"
#include "MYRTOS_ERROR_STATE.h"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Task AutoStart States:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
enum Task_AutoStart_t
{
	No_AutoStart,
	Yes_AutoStart
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Task States:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
enum Task_State_t
{
	Suspend_State,
	Waiting_State,
	Ready_State,
	Running_State
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Task Blocking or Not:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
enum Blocking_t
{
	Blocking_Disable,
	Blocking_Enable
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Task Timing Waiting Configuration:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct Task_Timing_Waiting_t
{
	enum Blocking_t Task_Blocking;
	unsigned int Ticks_Count;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Task Reference Configuration:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
typedef struct
{
	unsigned int  Task_Stack_Size;
	unsigned char Task_Priority;
	void (*PF_Task_Entry) (void);	//Pointer to Task C Function
	enum Task_AutoStart_t Task_AutoStart;

	unsigned int _S_PSP_Task;	//Not Entered by user
	unsigned int _E_PSP_Task;	//Not Entered by user
	unsigned int *Current_PSP_Task;	//This is the last address of PSP(SP) before we switch to another task

	char Task_Name[30];

	enum Task_State_t Task_State;	//Not Entered by user

	struct Task_Timing_Waiting_t Task_Timing_Waiting;
}Task_Ref_t;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Mutex States: we use binary semaphore
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
typedef enum
{
	Mutex_Blocked,
	Mutex_Released
}Binary_Semaphore_State_t;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Mutex Configuration:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
typedef struct
{
	void *Data;	//this to be generic to any data type not only to character
	unsigned int Data_Size;
	Task_Ref_t *Current_Task_User;
	Task_Ref_t *Next_Task_User;
	char Mutex_Name[30];
	Binary_Semaphore_State_t mutex_state;	//Not Entered by user
}Mutex_Configuration_t;


/*
 * ======================================================================
 * 			APIs Supported by "Scheduler"
 * ======================================================================
 */
MYRTOS_ES_t MYRTOS_init(void);
void MyRTOS_Task_Init(Task_Ref_t *Task_Ref_Config, unsigned int Stack_Size, void (*PF)(void), unsigned char Priority, char *Name);
MYRTOS_ES_t MyRTOS_Create_Task(Task_Ref_t *Task_Ref_Config);
MYRTOS_ES_t MyRTOS_Activate_Task(Task_Ref_t *Task_Ref_Config);
MYRTOS_ES_t MyRTOS_Terminate_Task(Task_Ref_t *Task_Ref_Config);
MYRTOS_ES_t MyRTOS_Start_OS(void);
MYRTOS_ES_t MyRTOS_Task_Wait(unsigned int No_Ticks, Task_Ref_t *Task_Ref_Config);
void MyRTOS_Mutex_Init(Mutex_Configuration_t *Mutex_Ref_Config, void *PayLoad, unsigned int PayLoad_Size, char *MUTEX_NAME);
MYRTOS_ES_t MyRTOS_Acquire_Mutex(Task_Ref_t *Task_Ref_Config, Mutex_Configuration_t *Mutex_Config);
void MyRTOS_Release_Mutex(Mutex_Configuration_t *Mutex_Config);

#endif /* INC_SCHEDULER_H_ */
