/*
 * Scheduler.c
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

/*
 * ------------
 * | Includes |
 * ------------
 */
#include "Scheduler.h"
#include "MYRTOS_FIFO.h"

//define a macro contain the maximum number of tasks
#define Max_Num_of_Tasks			100

//Ready Queue Buffer
FIFO_Buf_t Ready_Queue;
Task_Ref_t* Ready_Queue_FIFO[Max_Num_of_Tasks];

Task_Ref_t MyRTOS_IDLE_TASK;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//OS States:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
enum OS_Mode_t
{
	OS_Suspend,
	OS_Running
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//OS Control Configuration:
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct
{
	Task_Ref_t* OS_Tasks[Max_Num_of_Tasks];	//Task Scheduler Table

	unsigned int _S_MSP_OS;
	unsigned int _E_MSP_OS;
	unsigned int PSP_Task_Locator;

	unsigned int No_of_Active_Tasks;

	Task_Ref_t* Current_Task;
	Task_Ref_t* Next_Task;

	enum OS_Mode_t OS_State;

}OS_Control_t;


/*
 * Function: OS_SVC_Services
 * Usage:
 * 			--> it's used to execute specific OS Service
 */
void OS_SVC_Services(unsigned int *Stack_Pointer)
{
	unsigned char SVC_ID;
	SVC_ID = *((unsigned char *)(((unsigned char *)Stack_Pointer[6])-2));

	switch(SVC_ID)
	{
	case 0: //Activate Task
		break;

	case 1: //Terminate Task
		break;

	case 2:
		break;

	case 3:
		break;
	}
}


void PendSV_Handler(void)
{

}


void OS_SVC_Set(int SVC_ID)
{
	switch(SVC_ID)
	{
	case 0:	//Activate Task
		__asm("SVC #0x00");
		break;

	case 1:	//Terminate Task
		__asm("SVC #0x01");
		break;

	case 2:
		__asm("SVC #0x02");
		break;

	case 3:
		__asm("SVC #0x03");
		break;
	}
}

void MyRTOS_Create_MainStack(void)
{
	OS_Control_t._S_MSP_OS = (unsigned int)(&_estack);
	OS_Control_t._E_MSP_OS = (OS_Control_t._S_MSP_OS - Main_Stack_Size);

	//Aligned 8 bytes spaces between MSP (OS) and PSP (Tasks)
	OS_Control_t.PSP_Task_Locator = (OS_Control_t._E_MSP_OS - 8);
}

void IDLE_TASK_FUNC(void)
{
	while(1)
	{
		__asm("NOP");
	}
}

MYRTOS_ES_t MYRTOS_init(void)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//Updata OS Mode --> OS_Suspend
	OS_Control_t.OS_State = OS_Suspend;

	//Specify the Main Stack for OS
	MyRTOS_Create_MainStack();

	//Create OS Ready Queue
	if(FIFO_init(&Ready_Queue, Ready_Queue_FIFO, Max_Num_of_Tasks) != FIFO_no_error)
	{
		Local_enuErrorState = ES_Ready_Queue_Init_Error;
	}

	//Configure IDLE Task
	strcpy(MyRTOS_IDLE_TASK.Task_Name, "Idle_Task");
	MyRTOS_IDLE_TASK.Task_Priority = 255;
	MyRTOS_IDLE_TASK.PF_Task_Entry = IDLE_TASK_FUNC;
	MyRTOS_IDLE_TASK.Task_Stack_Size = 300;

	Local_enuErrorState = MyRTOS_Create_Task(&MyRTOS_IDLE_TASK);

	return Local_enuErrorState;
}

void MyRTOS_Create_Task_Stack(Task_Ref_t *Task_CFG)
{
	/*
	 * Task Frame:
	 * -----------------------------------
	 * |This Part is saved automatically |
	 * -----------------------------------
	 * |-------|
	 * |  xPSR |
	 * |  PC   |	//Next Task Instruction which will fetched
	 * |  LR   |	//Return register which is saved in CPU while Task1 running before Task Switching
	 * |  R12  |
	 * |  R3   |
	 * |  R2   |
	 * |  R1   |
	 * |  R0   |
	 * |-------|
	 * ----------------------------------------------
	 * |This Part is saved/restored by us (manually)|
	 * ----------------------------------------------
	 * |-------|
	 * |  R4   |
	 * |  R5   |
	 * |  R6   |
	 * |  R7   |
	 * |  R8   |
	 * |  R9   |
	 * |  R10  |
	 * |  R11  |
	 * |-------|
	 */

	Task_CFG->Current_PSP_Task = Task_CFG->_S_PSP_Task;

	Task_CFG->Current_PSP_Task--;
	//DUMMY xPSR --> you must put T = 1 to avoid Bus Fault (Thumb2 Technology)
	Task_CFG->Current_PSP_Task = 0x01000000;

	Task_CFG->Current_PSP_Task--;
	Task_CFG->Current_PSP_Task = (unsigned int)(Task_CFG->PF_Task_Entry);	//DUMMY PC

	Task_CFG->Current_PSP_Task--;
	Task_CFG->Current_PSP_Task = 0xFFFFFFFD;	//DUMMY LR
}

MYRTOS_ES_t MyRTOS_Create_Task(Task_Ref_t *Task_Ref_Config)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	/*
	 * --> SRAM
	 * -------------
	 * | _S_MSP_   |
	 * |    MSP    |
	 * | _E_MSP_   |
	 * -------------
	 * |  8 Byte   |
	 * -------------
	 * | _S_PSP_   |
	 * | Task1_PSP |
	 * | _E_PSP_   |
	 * -------------
	 * |  8 Byte   |
	 * -------------
	 * | _S_PSP_   |
	 * | Task2_PSP |
	 * | _E_PSP_   |
	 * -------------
	 * |  8 Byte   |
	 * -------------
	 * |   ....    |
	 * |   ....    |
	 * |   ....    |
	 * |   ....    |
	 * -------------
	 * | _eheap    |
	 * -------------
	 */

	// Check if task stack size exceeded the PSP stack size
	if(((OS_Control_t.PSP_Task_Locator - Task_Ref_Config->Task_Stack_Size) < (unsigned int)(&_eheap)))
	{
		Local_enuErrorState = ES_Error_Task_Exceeded_Stack_Size;
	}

	if(Local_enuErrorState == ES_NoError)
	{
		//Create Its Own PSP Stack
		Task_Ref_Config->_S_PSP_Task = OS_Control_t.PSP_Task_Locator;
		Task_Ref_Config->_E_PSP_Task = (Task_Ref_Config->_S_PSP_Task - Task_Ref_Config->Task_Stack_Size);

		//Aligned 8 bytes spaces between PSP (Task) and PSP (Other Task)
		OS_Control_t.PSP_Task_Locator = (Task_Ref_Config->_E_PSP_Task - 8);
	}

	//Initialize PSP Task Stack
	MyRTOS_Create_Task_Stack(Task_Ref_Config);

	//Task State Update --> Suspend State
	Task_Ref_Config->Task_State = Suspend_State;

	return Local_enuErrorState;
}
