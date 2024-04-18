/*
 * CortexMx_OS_Porting.c
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

/*
 * ------------
 * | Includes |
 * ------------
 */
#include "CortexMx_OS_Porting.h"



/* we make infinite loop for faults to avoid unpredictable thing if a fault is happen */
void HardFault_Handler (void)
{
	while(1)
	{
	}
}

void MemManage_Handler (void)
{
	while(1)
	{
	}
}

void BusFault_Handler (void)
{
	while(1)
	{
	}
}

void UsageFault_Handler (void)
{
	while(1)
	{
	}
}

/*
 * Function: SVC_Handler
 * Usage:
 * 		--> we jump to it when SVC interrupt is happened
		--> we use it to see which stack we use before the interrupt is happened
		--> we jump to anther function to do specific task depend on the SVC ID
		--> we make it 'naked' because we write it in assembly and we don't need to push
		 	 anything more in the stack to know to get SVC ID
 */
__attribute((naked)) void SVC_Handler(void)
{
	// Check which stack we use before stacking MSP or PSP
	__asm("TST LR, #0x4 \n\t"
			"ITE EQ \n\t"
			"MRSEQ R0, MSP \n\t"
			"MRSNE R0, PSP \n\t"
			"B OS_SVC_Services");
}


void HW_init(void)
{
	/*
	 * Initialize Clock Tree (RCC --> SysTick Timer & CPU) with 8 MHz
	 * ------------------------------------
	 * | Clock: 				8 MHz	  |
	 * | Time_one_count:		0.0125 us |
	 * ------------------------------------
	 * | num_counts	-----> 	1 Millisecond |
	 * | num_counts = 8000 count		  |
	 * ------------------------------------
	 */

	/*
	 * --------------------------------------------------------
	 * | CPU clock & SysTick Timer clock are 8 MHz by default |
	 * --------------------------------------------------------
	 */



	/*
	 * ------------------------------------------------------------------------------------------
	 * | Decrease PendSV Interrupt Priority to be less than or equal SysTick Interrupt Priority |
	 * | we will use __NVIC_SetPriority from core_cm3.h to set the priority                     |
	 * | we will get the priority of PendSV and SysTick from ARMCM3.h                           |
	 * | PendSV_IRQn = -2 --> 14          | SysTick_IRQn = -1 --> 15                            |
	 * ------------------------------------------------------------------------------------------
	 */
	IRQn_Type PendSV_Priority = PendSV_IRQn;
	IRQn_Type SysTick_Priority = SysTick_IRQn;
	__NVIC_SetPriority(PendSV_Priority, SysTick_Priority);


}

void Trigger_OS_PendSV(void)
{
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

unsigned int OS_Start_Ticker(void)
{
	/*
	 * Clock initialized with 8 MHz
	 * ------------------------------------
	 * | Clock: 				8 MHz	  |
	 * | Time_one_count:		0.0125 us |
	 * ------------------------------------
	 * | num_counts	-----> 	1 Millisecond |
	 * | num_counts = 8000 count		  |
	 * ------------------------------------
	 */
	return SysTick_Config(8000);
}

unsigned char SysTick_Led;
void SysTick_Handler(void)
{
	SysTick_Led ^= 1;

	/*
	 * -----------------------------------
	 * |Decide What task should run Next |
	 * -----------------------------------
	 */
	OS_Decide_What_Next();

	/*
	 * -----------------------------------------------------------
	 * |Trigger OS_PendSV (Switch Context/Restore for our Tasks) |
	 * -----------------------------------------------------------
	 */
	Trigger_OS_PendSV();
}
