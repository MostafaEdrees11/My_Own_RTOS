# RTOS Layers
```mermaid
graph TD;
	My_RTOS_Layers --> User_Tasks;
	My_RTOS_Layers --> Schedular;
	My_RTOS_Layers --> My_RTOSFIFO;
	My_RTOS_Layers --> CortexMx_OS_Porting;
```


## User Tasks
The user should write the tasks that he needs to execute them.
```mermaid
graph TD;
	User_Task --> its_own_priority;
	User_Task --> its_own_operations;
```

## Scheduler
```mermaid
graph TD;
	Scheduler --> APIs
	Scheduler --> Kernel
```

### APIs
This APIs that will provide to the user and it can use them to treat with tasks.
```mermaid
graph TD;
	APIs --> Create_Task
	APIs --> Start_OS
	APIs --> Terminate_Task
	APIs --> Activate_Task
	APIs --> ....
```


### Kernel

It will contain #the_scheduling_algorithm.

```mermaid
graph TD;
	Scheduler.c --> Scheduler.h;

	Scheduler.h --> CortexMx_OS_Porting.h;

	MYRTOS_FIFO.h --> stdio.h;
	MYRTOS_FIFO.h --> stdint.h;
	MYRTOS_FIFO.h --> Scheduler.h;
```

## My RTOS FIFO
```mermaid
graph TD;
	MYRTOS_FIFO --> FIFO_init;
	MYRTOS_FIFO --> FIFO_enqueue;
	MYRTOS_FIFO --> FIFO_dequeue;
	MYRTOS_FIFO --> FIFO_is_full;
	MYRTOS_FIFO --> FIFO_print;
```

## Porting
It will contain all things about CPU and SOC and if we change the CPU or SOC we should change the code in this file but we should fix the APIs of this file.

```mermaid
graph TD;
	CortexMx_OS_Porting --> Switch_CPU_Access_Level;
	CortexMx_OS_Porting --> HW_init;
	CortexMx_OS_Porting --> Start_Ticker;
	CortexMx_OS_Porting --> Ticker_Led;
	CortexMx_OS_Porting --> ....
```
___
# Tasks States

## Suspend State
* if we create a task it will enter suspend state.
* if we terminate a task it will enter suspend state.
* if we acquire a mutex and this mutex is taken by anthor task then our task will enter suspend state.

```mermaid
graph TD;
	Create_Task --> Suspend;
	Terminate_Task --> Suspend;
	Acquire_Mutex --> Suspend;
```
## Waiting State
* if we activate a task it will enter waiting state.
* if we acquire a mutex and this mutex is released then this task will enter waiting state.
* if we are in running state but anthor task in preempt us because it's high priority then our task will enter waiting state.

```mermaid
graph TD;
	Activate_Task --> Waiting
	Requested_Mutex_is_Released --> Waiting
	Another_Task_with_High_Priority --> Waiting
```
## Ready State
* if we are in waiting state but we have the highest priority then we enter ready state.
* if we have the same priority with another task then we enter the ready state when the another task is running for time slice.

```mermaid
graph TD;
	Task_with_Highest_Priority --> Ready;
	Round_Robin_with_another_Task --> Ready;
```

## Running State
* if we have the same priority with another task then we enter the running state when the another task is in ready state for time slice.
* if we are in ready state and we have the highest priority then we enter running state.

```mermaid
graph TD;
	Round_Robin_with_anthor_Task --> Running;
	Task_in_Ready_With_Highest_Priority --> Running;
```

## Conclusion 

```mermaid
graph TD;
	Create_Task --> Suspend;
	Terminate_Task --> Suspend;
	Acquire_Mutex --> Suspend;

	Suspend --> Activate_Task --> Waiting
	Suspend --> Requested_Mutex_is_Released --> Waiting
	Suspend --> Another_Task_with_High_Priority --> Waiting
	
	Waiting --> Task_with_Highest_Priority --> Ready;
	Waiting --> Round_Robin_with_another_Task --> Ready;

	Ready --> Round_Robin_with_anthor_Task --> Running;
	Ready --> Task_in_Ready_With_Highest_Priority --> Running;

	Running --> Task_Wait --> Suspend;
```


| Tasks |                      |                      |
| ----- | -------------------- | -------------------- |
|       | Task Tables          | Ready Queue          |
|       | ![[Task Tables.png]] | ![[Ready Queue.png]] |
___
# Hardware Initialize
we initialize CPU clock and  SysTick Timer clock
it will be by default 8 MHz
```mermaid
graph TD;
	HW_init --> RCC_Set_CLK;
```

___
# My RTOS Initialize
```mermaid
graph TD;
	MYRTOS_init --> Updata_OS_Mode
	MYRTOS_init --> Specify_Main_Stack_OS
	MYRTOS_init --> Create_OS_Ready_Queue
	MYRTOS_init --> Configure_IDLE_Task
```

