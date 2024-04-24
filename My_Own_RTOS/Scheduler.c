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

//Idle Task
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
 *          ^
 * Priority |
 *          |                 -------                      -------
 *          |                 | SVC |                      | SVC |		<--- Reorder Scheduler Table, Update Ready Queue
 *          |                 -------                      -------
 *          |                |       |                    |       |
 *          |                |       |                    |       |
 *          |                |       |                    |       |
 *          |                |       |                    |       |
 *          |                |       |                    |       |
 *          |                |       |                    |       |
 *          |                |       |                    |       |
 *          |                |        ----------          |        ----------
 *          |                |        | PendSV |          |        | PendSV |		<--- Switch Context/Restore for current & next task
 *          |                |        ----------          |        ----------
 *          |                |                  |         |                  |
 *          |                |                  |         |                  |
 *          |                |                  |         |                  |
 *          |                |                  |         |                  |
 *          |                |                  |         |                  |
 *          |                |                  |         |                  |
 *          |       ---------                    ---------                    ---------
 *          |       | Task1 |                    | Task2 |                    | Task1 |
 *          |       ---------                    ---------                    ---------
 *          |
 *          |
 *          |
 *          |________________________________________________________________________________________ Time
 *
 *          				<< Task1 Priority == Task2 Priority    >>
 *          				<< Round Robin between Task1 and Task2 >>
 *          				<< This when we Activate the two tasks >>
 */

/*
 *          ^
 * Priority |
 *          |                 -----------                      -----------
 *          |                 | SysTick |                      | SysTick |		<--- Decide What Current and Next
 *          |                 -----------                      -----------
 *          |                |           |                    |           |
 *          |                |           |                    |           |
 *          |                |           |                    |           |
 *          |                |           |                    |           |
 *          |                |           |                    |           |
 *          |                |           |                    |           |
 *          |                |           |                    |           |
 *          |                |            ----------          |            ----------
 *          |                |            | PendSV |          |            | PendSV |		<--- Switch Context/Restore for current & next task
 *          |                |            ----------          |            ----------
 *          |                |                      |         |                      |
 *          |                |                      |         |                      |
 *          |                |                      |         |                      |
 *          |                |                      |         |                      |         ...
 *          |                |                      |         |                      |         |
 *          |                |                      |         |                      |         |
 *          |       ---------                        ---------                        ---------
 *          |       | Task1 |                        | Task2 |                        | Task1 |
 *          |       ---------                        ---------                        ---------
 *          |
 *          |
 *          |
 *          |________________________________________________________________________________________ Time
 *                           ^                                ^                                ^
 *                           |                                |                                |
 *                           1ms (Ticker)                     1ms (Ticker)                     1ms (Ticker)
 *
 *          				<< Task1 Priority == Task2 Priority    >>
 *          				<< Round Robin between Task1 and Task2 >>
 *          				<< Each Task work for 1ms              >>
 */

/*
 * Function Name : PendSV_Handler
 * Function [IN] : none
 * Function [OUT]: none
 * Usage         : it's PendSV handler and we use it to make context switch to the current task
 *                 and context restore for the next task
 */
__attribute ((naked)) void PendSV_Handler(void)
{
	//Switch Context/Restore of our tasks

	/*
	 * ----------------------------------------
	 * | Save The Context of the current task |
	 * ----------------------------------------
	 */

	/*
	 * 1. Get the Current_PSP from CPU registers
	 * |-------|
	 * |  xPSR |
	 * |  PC   |
	 * |  LR   |
	 * |  R12  |
	 * |  R3   |
	 * |  R2   |
	 * |  R1   |
	 * |  R0   |	<-- Current PSP
	 * |-------|
	 */
	OS_Get_PSP_Val(OS_Control_t.Current_Task->Current_PSP_Task);

	/*
	 * 2. Save the registers from R4 to R11
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
	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R4" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R4

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R5" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R5

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R6" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R6

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R7" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R7

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R8" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R8

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R9" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R9

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R10" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R10

	OS_Control_t.Current_Task->Current_PSP_Task--;
	__asm volatile ("MOV %[OUT], R11" : [OUT] "=r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R11

	/*
	 * 3. Save the Current Value of PSP --> it Saved in Current_PSP_Task
	 * |-------|
	 * |  R4   |
	 * |  R5   |
	 * |  R6   |
	 * |  R7   |
	 * |  R8   |
	 * |  R9   |
	 * |  R10  |
	 * |  R11  |		<-- Current_PSP_Task
	 * |-------|
	 */

	/*
	 * -----------------------------------------------------------------------------------------------
	 * |=============================================================================================|
	 * -----------------------------------------------------------------------------------------------
	 */



	/*
	 * ----------------------------------------
	 * | Restore The Context of the Next task |
	 * ----------------------------------------
	 */
	if(OS_Control_t.Next_Task != NULL)
	{
		OS_Control_t.Current_Task = OS_Control_t.Next_Task;
		OS_Control_t.Next_Task = NULL;
	}

	/*
	 * 1. Write the values of registers from memory to CPU registers
	 * |-------|
	 * |  R4   |
	 * |  R5   |
	 * |  R6   |
	 * |  R7   |
	 * |  R8   |
	 * |  R9   |
	 * |  R10  |
	 * |  R11  |		<-- Current_PSP_Task
	 * |-------|
	 */
	__asm volatile ("MOV R11, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R11
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R10, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R10
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R9, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R9
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R8, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R8
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R7, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R7
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R6, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R6
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R5, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R5
	OS_Control_t.Current_Task->Current_PSP_Task++;

	__asm volatile ("MOV R4, %[IN]" : : [IN] "r" ((*OS_Control_t.Current_Task->Current_PSP_Task))); //R4
	OS_Control_t.Current_Task->Current_PSP_Task++;

	/*
	 * 2.Set PSP with Current_PSP_Task
	 */
	OS_Set_PSP_Val(OS_Control_t.Current_Task->Current_PSP_Task);

	/*
	 * 3.The other registers will restored automatically by CPU
	 * |-------| |-----------------------------|
	 * |  xPSR | |*****************************|
	 * |  PC   | |*****************************|
	 * |  LR   | |*****************************|
	 * |  R12  | |<CPU read them automatically>|
	 * |  R3   | |*****************************|
	 * |  R2   | |*****************************|
	 * |  R1   | |*****************************|
	 * |  R0   |	<-- Current PSP		<-- PSP
	 * |-------| |-----------------------------|
	 * |  R4   | |*****************************|
	 * |  R5   | |*****************************|
	 * |  R6   | |*****************************|
	 * |  R7   | |***<we read them manually>***|
	 * |  R8   | |*****************************|
	 * |  R9   | |*****************************|
	 * |  R10  | |*****************************|
	 * |  R11  | |*****************************|
	 * |-------| |-----------------------------|
	 */

	/*
	 * 4. Branch to LR to return from Interrupt handler
	 * LR --> contain EXC_RETURN Code
	 */
	__asm("BX LR");
}

/*
 * Function Name : MyRTOS_Create_MainStack
 * Function [IN] : none
 * Function [OUT]: none
 * Usage         : it's used to limit the bounds of Main Stack that we will use it for OS & Interrupts
 */
void MyRTOS_Create_MainStack(void)
{
	OS_Control_t._S_MSP_OS = (unsigned int)(&_estack);
	OS_Control_t._E_MSP_OS = (OS_Control_t._S_MSP_OS - Main_Stack_Size);

	//Aligned 8 bytes spaces between MSP (OS) and PSP (Tasks)
	OS_Control_t.PSP_Task_Locator = (OS_Control_t._E_MSP_OS - 8);
}

unsigned char IDLE_Task_Led;
/*
 * Function Name : IDLE_TASK_FUNC
 * Function [IN] : none
 * Function [OUT]: none
 * Usage         : it's the function of the idle task that will executed when no task is running
 */
void IDLE_TASK_FUNC(void)
{
	while(1)
	{
		IDLE_Task_Led ^= 1;
		__asm("NOP");
	}
}

/*
 * Function Name : MYRTOS_init
 * Function [IN] : none
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to initialize RTOS like create main task & configure idle task
 */
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

/*
 * Function Name : MyRTOS_Create_Task_Stack
 * Function [IN] : it takes a pointer to task that we need to create a task for it
 * Function [OUT]: none
 * Usage         : it's used to limit the bounds of Process Stack that we will use it for this task
 */
void MyRTOS_Create_Task_Stack(Task_Ref_t *Task_Ref_CFG)
{
	/*
	 * Task Frame:
	 * --------------------------------------------
	 * |This Part is saved/restored automatically |
	 * --------------------------------------------
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

	Task_Ref_CFG->Current_PSP_Task = (unsigned int *)(Task_Ref_CFG->_S_PSP_Task);

	Task_Ref_CFG->Current_PSP_Task--;
	*(Task_Ref_CFG->Current_PSP_Task) = 0x01000000;	//DUMMY xPSR --> you must put T = 1 to avoid Bus Fault (Thumb2 Technology)

	Task_Ref_CFG->Current_PSP_Task--;
	*(Task_Ref_CFG->Current_PSP_Task) = (unsigned int)(Task_Ref_CFG->PF_Task_Entry);	//DUMMY PC

	Task_Ref_CFG->Current_PSP_Task--;
	*(Task_Ref_CFG->Current_PSP_Task) = 0xFFFFFFFD;	//DUMMY LR --> (EXECUTION RETURN CODE --> Thread Mode, PSP)

	//Still 13 General Purpose Register --> We dummy them to 0
	for(int i = 0; i < 13; i++)
	{
		Task_Ref_CFG->Current_PSP_Task--;
		*(Task_Ref_CFG->Current_PSP_Task) = 0;
	}

}

/*
 * Function Name : MyRTOS_Create_Task
 * Function [IN] : it takes a pointer to task configuration that we need to create task it
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : we use it to create task such as create its stack and configure its state
 */
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

	//Add Task to Scheduler Table
	OS_Control_t.OS_Tasks[OS_Control_t.No_of_Active_Tasks++] = Task_Ref_Config;

	return Local_enuErrorState;
}

/*
 * Enumeration Name: SVC_ID_t
 * Usage:          : it has all cases of SVC IDs
 */
typedef enum
{
	SVC_Activate_Task = 1,
	SVC_Terminate_Task,
	SVC_Task_Waiting_Time,
	SVC_Acquire_Mutex,
	SVC_Release_Mutex
}SVC_ID_t;

/*
 * Function Name : Bubble_Sort_Tasks
 * Function [IN] : none
 * Function [OUT]: none
 * Usage         : it's used to sort the tasks on scheduler table based on the priority of each task
 */
void Bubble_Sort_Tasks(void)
{
	unsigned int i, j, Num_Tasks;
	Task_Ref_t *Temp;

	Num_Tasks = OS_Control_t.No_of_Active_Tasks;

	for(i = 0; i < Num_Tasks; i++)
	{
		for(j = 0; j < Num_Tasks - i - 1; j++)
		{
			if(OS_Control_t.OS_Tasks[j]->Task_Priority > OS_Control_t.OS_Tasks[j+1]->Task_Priority)
			{
				Temp = OS_Control_t.OS_Tasks[j];
				OS_Control_t.OS_Tasks[j] = OS_Control_t.OS_Tasks[j+1];
				OS_Control_t.OS_Tasks[j+1] = Temp;
			}
		}
	}
}


/*
 * Function Name : MyRTOS_Update_Scheduler_Tabel
 * Function [IN] : none
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to update scheduler table and rearrange the tasks in the table
 */
MYRTOS_ES_t MyRTOS_Update_Scheduler_Tabel(void)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//Sort Scheduler Table (OS_Control --> Tasks[100]) --> with Bubble Sort
	//Base on Priority --> (high priority(low number) then low priority(high number))
	Bubble_Sort_Tasks();

	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_Update_Ready_Queue
 * Function [IN] : none
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to update ready queue and add to it the tasks that should be in ready state
 */
MYRTOS_ES_t MyRTOS_Update_Ready_Queue(void)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	Task_Ref_t *Top_Ready_Queue = NULL;
	unsigned int i = 0;

	Task_Ref_t *P_Curr_Task = NULL;
	Task_Ref_t *P_Next_Task = NULL;

	//Free Ready Queue
	while(FIFO_Dequeue_Item(&Ready_Queue, &Top_Ready_Queue) != FIFO_empty);

	//Update Ready Queue
	while(i < OS_Control_t.No_of_Active_Tasks)
	{
		P_Curr_Task = OS_Control_t.OS_Tasks[i];
		P_Next_Task = OS_Control_t.OS_Tasks[i+1];

		/*
		 * ------------------
		 * |Scheduler Table:|
		 * ------------------
		 *
		 * |----------|----------|
		 * | Task1 (1)|  suspend |
		 * |----------|----------|
		 * | Task2 (2)|  waiting |
		 * |----------|----------|
		 * | Task3 (2)|  waiting |
		 * |----------|----------|
		 * | Task4 (3)|  waiting |
		 * |----------|----------|
		 * |    0     |    0     |
		 * |----------|----------|
		 * |    0     |    0     |
		 * |----------|----------|
		 */

		//we are care now with tasks that are in waiting state not suspend state
		if(P_Curr_Task->Task_State != Suspend_State)
		{
			//This if we reach to the end of the scheduler table
			if(P_Next_Task->Task_State == Suspend_State)
			{
				FIFO_Enqueue_Item(&Ready_Queue, P_Curr_Task);
				P_Curr_Task->Task_State = Ready_State;
				break;
			}

			if(P_Curr_Task->Task_Priority < P_Next_Task->Task_Priority)
			{
				//This if the next task is low priority from the current then we push current to ready queue
				FIFO_Enqueue_Item(&Ready_Queue, P_Curr_Task);
				P_Curr_Task->Task_State = Ready_State;
				break;
			}
			else if(P_Curr_Task->Task_Priority == P_Next_Task->Task_Priority)
			{
				//This if the next task is equal the current task in priority then we push current to ready queue
				//and we will continue to the relation of next task with its next
				FIFO_Enqueue_Item(&Ready_Queue, P_Curr_Task);
				P_Curr_Task->Task_State = Ready_State;
			}
			else if(P_Curr_Task->Task_Priority > P_Next_Task->Task_Priority)
			{
				//This can't happen because we sort the scheduler table with bubble sort
				Local_enuErrorState = ES_Error_Bubble_Sort;
			}
		}

		i++;
	}

	return Local_enuErrorState;
}

/*
 * Function Name : OS_Decide_What_Next
 * Function [IN] : none
 * Function [OUT]: none
 * Usage         : it's used to decide which task that should run next the current task
 */
void OS_Decide_What_Next(void)
{
	//This in case The Queue is empty and OS_Control_t.Current_Task->Task_State != Suspend_State
	//This happen when we have only one task and this task is interrupting by svc
	//we need to continue in running it
	if(Ready_Queue.count == 0 && OS_Control_t.Current_Task->Task_State != Suspend_State)
	{
		OS_Control_t.Current_Task->Task_State = Running_State;

		//add Task to Ready Queue to run it till the task is terminate
		FIFO_Enqueue_Item(&Ready_Queue, OS_Control_t.Current_Task);
		OS_Control_t.Next_Task = OS_Control_t.Current_Task;
	}
	else
	{
		//dequeue the top of ready queue because this is should running next
		FIFO_Dequeue_Item(&Ready_Queue, &OS_Control_t.Next_Task);
		OS_Control_t.Next_Task->Task_State = Running_State;

		//check if the next task priority is equal the current task priority to work with Round Robin Algorithm
		//check if the user doesn't terminate the current task so we will run it
		if((OS_Control_t.Current_Task->Task_Priority == OS_Control_t.Next_Task->Task_Priority) && (OS_Control_t.Current_Task->Task_State != Suspend_State))
		{
			//enqueue the current task in the Ready Queue so that the other task will be on top and current will be after it
			//we do this because we run with Round Robin Algorithm
			FIFO_Enqueue_Item(&Ready_Queue, OS_Control_t.Current_Task);
			OS_Control_t.Current_Task->Task_State = Ready_State;
		}
	}
}

/*
 * Function Name : OS_SVC_Services
 * Function [IN] : it tasks a pointer to the start address memory after we do switch context
 *                 we will use it to get SVC ID and we take it from R0
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to determine the SVC ID then call the SVC handler with correct id
 */
MYRTOS_ES_t OS_SVC_Services(unsigned int *Stack_Frame_Pointer)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	unsigned char SVC_ID;
	SVC_ID = *((unsigned char *)(((unsigned char *)Stack_Frame_Pointer[6])-2));

	switch(SVC_ID)
	{
	case SVC_Activate_Task:
	case SVC_Terminate_Task:
		//Update Scheduler Table
		Local_enuErrorState = MyRTOS_Update_Scheduler_Tabel();
		if(Local_enuErrorState != ES_NoError)
			while(1);

		//Update Ready Queue
		Local_enuErrorState = MyRTOS_Update_Ready_Queue();
		if(Local_enuErrorState != ES_NoError)
			while(1);

		//OS is in Running State or not
		if(OS_Control_t.OS_State == OS_Running)
		{
			if(strcmp(OS_Control_t.Current_Task->Task_Name, "Idle_Task") != 0)
			{
				//Decide What task should run Next
				OS_Decide_What_Next();

				//Trigger OS_PendSV (Switch Context/Restore for our Tasks)
				Trigger_OS_PendSV();
			}
		}
		break;

	case SVC_Task_Waiting_Time:
		//Update Scheduler Table
		Local_enuErrorState = MyRTOS_Update_Scheduler_Tabel();
		if(Local_enuErrorState != ES_NoError)
			while(1);

		//Update Ready Queue
		Local_enuErrorState = MyRTOS_Update_Ready_Queue();
		if(Local_enuErrorState != ES_NoError)
			while(1);
		break;

	case SVC_Acquire_Mutex:
		//Update Scheduler Table
		Local_enuErrorState = MyRTOS_Update_Scheduler_Tabel();
		if(Local_enuErrorState != ES_NoError)
			while(1);

		//Update Ready Queue
		Local_enuErrorState = MyRTOS_Update_Ready_Queue();
		if(Local_enuErrorState != ES_NoError)
			while(1);
		break;

	case SVC_Release_Mutex:
		//Update Scheduler Table
		Local_enuErrorState = MyRTOS_Update_Scheduler_Tabel();
		if(Local_enuErrorState != ES_NoError)
			while(1);

		//Update Ready Queue
		Local_enuErrorState = MyRTOS_Update_Ready_Queue();
		if(Local_enuErrorState != ES_NoError)
			while(1);
		break;
	}

	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_OS_SVC_Set
 * Function [IN] : it takes the ID of service that we need the OS do it
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to call SVC with specific to execute specific function
 */
MYRTOS_ES_t MyRTOS_OS_SVC_Set(SVC_ID_t svc_id)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//we will use svc to reorder Task_Scheduler and update Ready_Queue
	switch(svc_id)
	{
	case SVC_Activate_Task:
		__asm("SVC #0x01");
		break;

	case SVC_Terminate_Task:
		__asm("SVC #0x02");
		break;

	case SVC_Task_Waiting_Time:
		__asm("SVC #0x03");
		break;

	case SVC_Acquire_Mutex:
		__asm("SVC #0x04");
		break;

	case SVC_Release_Mutex:
		__asm("SVC #0x05");
		break;
	}

	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_Task_Init
 * Function [IN] : it takes a pointer to task configuration and its cofiguration parameters
 * Function [OUT]: none
 * Usage         : it's used to initialize a task
 */
void MyRTOS_Task_Init(Task_Ref_t *Task_Ref_Config, unsigned int Stack_Size, void (*PF)(void), unsigned char Priority, char *Name)
{
	Task_Ref_Config->Task_Stack_Size = Stack_Size;
	Task_Ref_Config->Task_Priority = Priority;

	Task_Ref_Config->PF_Task_Entry = PF;

	strcpy(Task_Ref_Config->Task_Name, Name);
}

/*
 * Function Name : MyRTOS_Activate_Task
 * Function [IN] : it takes a pointer to task configuration that we need to Activate it
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to activate task by adding it in waiting state then call SVC
 */
MYRTOS_ES_t MyRTOS_Activate_Task(Task_Ref_t *Task_Ref_Config)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//Task enter waiting state when we activate it
	Task_Ref_Config->Task_State = Waiting_State;

	//set svc interrupt to activate the task
	MyRTOS_OS_SVC_Set(SVC_Activate_Task);


	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_Terminate_Task
 * Function [IN] : it takes a pointer to task configuration that we need to Activate it
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to terminate task by adding it in suspend state then call SVC
 */
MYRTOS_ES_t MyRTOS_Terminate_Task(Task_Ref_t *Task_Ref_Config)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//Task enter suspend state when we terminate it
	Task_Ref_Config->Task_State = Suspend_State;

	//set svc interrupt to terminate the task
	MyRTOS_OS_SVC_Set(SVC_Terminate_Task);

	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_Start_OS
 * Function [IN] : none
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to start os by set it in running state and ....
 */
MYRTOS_ES_t MyRTOS_Start_OS(void)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;
	unsigned int Function_State = 1;

	//Enter the OS in Running Mode instead of Suspend Mode
	OS_Control_t.OS_State = OS_Running;

	//Set Default Task --> IDLE Task
	OS_Control_t.Current_Task = &MyRTOS_IDLE_TASK;

	//Activate IDLE Task --> Run IDLE Task
	Local_enuErrorState = MyRTOS_Activate_Task(&MyRTOS_IDLE_TASK);

	//Start Ticker --> 1ms
	Function_State = OS_Start_Ticker();
	if(Function_State)
		Local_enuErrorState = ES_Error_SysTick_counting;

	//Set PSP with PSP of Current Task
	OS_Set_PSP_Val(OS_Control_t.Current_Task->Current_PSP_Task);

	//Set SP shadow to PSP instead of MSP
	OS_Set_SP_shadowto_PSP;

	//Switch from Privileged to Unprivileged
	OS_Switch_Privileged_to_Unprivileged;

	//Run Current Task
	OS_Control_t.Current_Task->PF_Task_Entry();

	return Local_enuErrorState;
}


/*
 * Task State:
 *
 * -----------           Task Wait (No_Ticks)           -----------         No_Ticks = 0                    -----------
 * | Running |  	------------------------------>		| Suspend |		------------------------------> 	| Waiting |
 * -----------           Task Blocking Enable           -----------         Task Blocking Disable           -----------
 *
 */

/*
 * Function Name : MyRTOS_Task_Wait
 * Function [IN] : it takes the number of tick and pointer to the task
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to block the task in suspend state for a period of time
 */
MYRTOS_ES_t MyRTOS_Task_Wait(unsigned int No_Ticks, Task_Ref_t *Task_Ref_Config)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//Task will enter Suspend state
	Task_Ref_Config->Task_State = Suspend_State;

	//Enable Blocking and fill the number of ticks
	Task_Ref_Config->Task_Timing_Waiting.Task_Blocking = Blocking_Enable;
	Task_Ref_Config->Task_Timing_Waiting.Ticks_Count = No_Ticks;

	//Terminate the task now
	MyRTOS_Terminate_Task(Task_Ref_Config);

	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_Update_Waiting_Time
 * Function [IN] : none
 * Function [OUT]: none
 * Usage         : it's used to see if the time of blocking task is terminated or not
 */
void MyRTOS_Update_Waiting_Time(void)
{
	unsigned int i;

	//loop for task that in suspend state and it has its own waiting time
	for(i = 0; i < OS_Control_t.No_of_Active_Tasks; i++)
	{
		if(OS_Control_t.OS_Tasks[i]->Task_State == Suspend_State)
		{
			if(OS_Control_t.OS_Tasks[i]->Task_Timing_Waiting.Task_Blocking == Blocking_Enable)
			{
				OS_Control_t.OS_Tasks[i]->Task_Timing_Waiting.Ticks_Count--;

				//if the waiting time is finish we will disable blocking and enter the task in waiting state
				if(OS_Control_t.OS_Tasks[i]->Task_Timing_Waiting.Ticks_Count == 0)
				{
					OS_Control_t.OS_Tasks[i]->Task_Timing_Waiting.Task_Blocking = Blocking_Disable;
					OS_Control_t.OS_Tasks[i]->Task_State = Waiting_State;

					MyRTOS_OS_SVC_Set(SVC_Task_Waiting_Time);
				}
			}
		}
	}
}

/*
 * Function Name : MyRTOS_Mutex_Init
 * Function [IN] : it takes pointer to Mutex and it's configuration
 * Function [OUT]: none
 * Usage         : it's used to initialize the mutex with the send configuration
 */
void MyRTOS_Mutex_Init(Mutex_Configuration_t *Mutex_Ref_Config, void *PayLoad, unsigned int PayLoad_Size, char *MUTEX_NAME)
{
	Mutex_Ref_Config->Current_Task_User = NULL;
	Mutex_Ref_Config->Next_Task_User = NULL;

	Mutex_Ref_Config->Data = PayLoad;
	Mutex_Ref_Config->Data_Size = PayLoad_Size;

	Mutex_Ref_Config->mutex_state = Mutex_Released;

	strcpy(Mutex_Ref_Config->Mutex_Name, MUTEX_NAME);
}

/*
 * ---------
 * | Task1 |	---> Running 	------> Acquired a Mutex1 			---> Mutex is blocked now
 * ---------
 *
 * ---------
 * | Task2 |	---> Running 	------> Acquire the Mutex1 but it's blocked		---> Suspend
 * ---------
 *
 * after some time ----> Mutex1 is released by Task1
 * then we need to update the scheduler table and enter Task2 in waiting state
 *
 * ---------
 * | Task2 |	---> Waiting	---> Ready		---> Running 	---> so now it can use the shared data
 * ---------
 */

/*
 * Function Name : MyRTOS_Acquire_Mutex
 * Function [IN] : it takes a pointer to the task and pointer to the Mutex
 * Function [OUT]: it's return the error state of function to check with it if any error happens
 * Usage         : it's used to acquire the mutex by specific task
 */
MYRTOS_ES_t MyRTOS_Acquire_Mutex(Task_Ref_t *Task_Ref_Config, Mutex_Configuration_t *Mutex_Config)
{
	MYRTOS_ES_t Local_enuErrorState = ES_NoError;

	//if the mutex is released and is not taken by any task
	if(Mutex_Config->Current_Task_User == NULL || Mutex_Config->mutex_state == Mutex_Released)
	{
		Mutex_Config->Current_Task_User = Task_Ref_Config;
		Mutex_Config->mutex_state = Mutex_Blocked;
	}
	else	//if the mutex taken and used by the current task
	{
		if(Mutex_Config->Next_Task_User == NULL) //There is no pending Task for this mutex
		{
			Mutex_Config->Next_Task_User = Task_Ref_Config;

			//task will enter the suspend state till the mutex is released
			Task_Ref_Config->Task_State = Suspend_State;

			//terminate the task
			MyRTOS_OS_SVC_Set(SVC_Terminate_Task);

		}
		else	//there is a pending task need this mutex
		{
			Local_enuErrorState = ES_Error_Many_User_Mutex;
		}
	}

	return Local_enuErrorState;
}

/*
 * Function Name : MyRTOS_Release_Mutex
 * Function [IN] : it takes a pointer to the mutex
 * Function [OUT]: none
 * Usage         : it's used to release a mutex
 */
void MyRTOS_Release_Mutex(Mutex_Configuration_t *Mutex_Config)
{
	if(Mutex_Config->Current_Task_User == NULL || Mutex_Config->mutex_state == Mutex_Blocked)
	{
		Mutex_Config->Current_Task_User = Mutex_Config->Next_Task_User;
		Mutex_Config->Next_Task_User = NULL;

		Mutex_Config->mutex_state = Mutex_Released;

		Mutex_Config->Current_Task_User->Task_State = Waiting_State;

		MyRTOS_Activate_Task(Mutex_Config->Current_Task_User);
	}
}

/*
 * 										< Example on Priority Inversion>
 *          ^
 * Priority |
 *          |
 *          |
 *          |     --------------------                                       ---------------------                                                                        ^ low Priority
 *  Task1   |     | Task1 "Completed"|  <--- Acquire Mutex                   | Task1 "Completed" |  <--- Release the Mutex                                                |
 *          |     --------------------                                       ---------------------                                                                        |
 *          |                        |                                       |                    |                                                                       |
 *          |                        |                                       |                    |                                                                       |
 *          |                        |                                       |                    |                                                                       |
 *          |                        |                                       |                    |                                                                       |
 *          |                        ---------                       ---------                    |
 *  Task2   |                        | Task2 |                       | Task2 |                    |
 *          |                        ---------                       ---------                    |
 *          |                                |                       |                            |
 *          |                                |                       |                            |
 *          |                                |                       |                            |
 *          |                                |                       |                            |
 *          |                                -------------------------                            ------------------
 *  Task3   |                                | Task3 "Not Completed" | <--- Acquire same Mutex    | Complete Task3 |   <--- Now, we can acquire the same mutex
 *          |                                -------------------------                            ------------------
 *          |
 *          |______________________________________________________________________________________________________________________________________________________ Time
 *                                                                   ^                            ^
 *                                                                   |                            |
 *                                                                   |----------------------------|
 *                                                                 <Priority Inversion Latency Time >
 *
 *
 */


/*
 * 																		< Example on Dead Lock>
 *          ^
 * Priority |
 *          |
 *          |
 *          |     -------------------------------------------------                                                                                   ----------------------------------------------------------------
 *  Task1   |     | Task1 <--- Acquire Mutex1 then Activate Task3 |                                                                                   | Task1 <--- Acquire Mutex2 but can't because it used by Task3 |
 *          |     -------------------------------------------------                                                                                   ----------------------------------------------------------------
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     |                                                                                  |
 *          |                                                     -----------------------------------------------------------------------------------
 *  Task3   |                                                     | Task3 <--- Acquire Mutex2 then Acquire Mutex1 but can't because it used by Task1|
 *          |                                                     |----------------------------------------------------------------------------------
 *          |
 *          |______________________________________________________________________________________________________________________________________________________ Time
 *
 *
 *
 */
