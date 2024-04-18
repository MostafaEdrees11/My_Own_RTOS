/*
 * CortexMx_OS_Porting.h
 *
 *  Created on: Apr 14, 2024
 *      Author: Mostafa Edrees
 */

#ifndef INC_CORTEXMX_OS_PORTING_H_
#define INC_CORTEXMX_OS_PORTING_H_

/*
 * ------------
 * | Includes |
 * ------------
 */
#include "core_cm3.h"

extern unsigned int _estack;
extern unsigned int _eheap;


/*
 * ======================================================================
 * 			APIs Supported by "CortexMx OS Porting"
 * ======================================================================
 */
void HW_init(void);
void Trigger_OS_PendSV(void);
unsigned int OS_Start_Ticker(void);


/*
 * ======================================================================
 * 			Macros Supported by "CortexMx OS Porting"
 * ======================================================================
 */

#define Main_Stack_Size		3072		//Main Stack Size = 3 KB

/*
 * Function: OS_Set_PSP_Val
 * How:
 * 		--> Move address of psp that we have to general purpose register 'r0'
		--> Move the value of R0 to PSP by using MSR
 */
#define OS_Set_PSP_Val(address)							__asm volatile("MOV R0, %[IN] \n\t MSR PSP, R0" : : [IN] "r" (address))

/*
 * Function: OS_Get_PSP_Val
 * How:
 * 		--> Move the value of PSP to general purpose register 'r0' by using MRS
		--> Move the value of R0 to the variable that we have
 */
#define OS_Get_PSP_Val(address)							__asm volatile("MRS R0, PSP \n\t MOV %[OUT], R0" : [OUT] "=r" (address))

/*
 * Function: OS_Set_SP_shadowto_PSP
 * Brief How: Set Bit1 @ Control Register
 * How:
 * 		--> Move control register to general purpose register 'r0'
 * 		--> Write 0x2 on general purpose register 'r1'
 * 		--> Logical or between R0 and R1(0x2) to set the second bit
		--> Move the value of R0 after set the bit to control register
 */
#define OS_Set_SP_shadowto_PSP							__asm volatile("MRS R0, CONTROL \n\t MOV R1, #0x02 \n\t \
																		ORR R0, R0, R1 \n\t MSR CONTROL, R0")

/*
 * Function: OS_Set_SP_shadowto_MSP
 * Brief How: Clear Bit1 @ Control Register
 * How:
 * 		--> Move control register to general purpose register 'r0'
 * 		--> Write 0x5 on general purpose register 'r1'
 * 		--> Logical and between R0 and R1(0x5) to clear the second bit
		--> Move the value of R0 after clear the bit to control register
 */
#define OS_Set_SP_shadowto_MSP							__asm volatile("MRS R0, CONTROL \n\t MOV R1, #0x05 \n\t \
																		AND R0, R0, R1 \n\t MSR CONTROL, R0")

/*
 * Function: OS_Switch_Privileged_to_Unprivileged
 * Brief How: Set Bit0 @ Control Register
 * How:
 * 		--> Move control register to general purpose register 'r0'
 * 		--> Write 0x1 on general purpose register 'r1'
 * 		--> Logical or between R0 and R1(0x1) to set the first bit
		--> Move the value of R0 after set the bit to control register
 */
#define OS_Switch_Privileged_to_Unprivileged			__asm volatile("MRS R0, CONTROL \n\t MOV R1, #0x01 \n\t \
																		ORR R0, R0, R1 \n\t MSR CONTROL, R0")

/*
 * Function: OS_Switch_Unprivileged_to_Privileged
 * Brief How: Clear Bit0 @ Control Register
 * How:
 * 		--> Move control register to general purpose register 'r0'
 * 		--> Write 0x6 on general purpose register 'r1'
 * 		--> Logical and between R0 and R1(0x6) to clear the first bit
		--> Move the value of R0 after clear the bit to control register
 */
#define OS_Switch_Unprivileged_to_Privileged			__asm volatile("MRS R0, CONTROL \n\t MOV R1, #0x06 \n\t \
																		AND R0, R0, R1 \n\t MSR CONTROL, R0")

#endif /* INC_CORTEXMX_OS_PORTING_H_ */
