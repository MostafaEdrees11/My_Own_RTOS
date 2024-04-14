################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../My_Own_RTOS/CortexMx_OS_Porting.c \
../My_Own_RTOS/MYRTOS_FIFO.c \
../My_Own_RTOS/Scheduler.c 

OBJS += \
./My_Own_RTOS/CortexMx_OS_Porting.o \
./My_Own_RTOS/MYRTOS_FIFO.o \
./My_Own_RTOS/Scheduler.o 

C_DEPS += \
./My_Own_RTOS/CortexMx_OS_Porting.d \
./My_Own_RTOS/MYRTOS_FIFO.d \
./My_Own_RTOS/Scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
My_Own_RTOS/CortexMx_OS_Porting.o: ../My_Own_RTOS/CortexMx_OS_Porting.c
	arm-none-eabi-gcc -gdwarf-2 "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/My_Own_RTOS/inc" -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/CMSIS_V5" -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/STM32_F103C6_Drivers/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"My_Own_RTOS/CortexMx_OS_Porting.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
My_Own_RTOS/MYRTOS_FIFO.o: ../My_Own_RTOS/MYRTOS_FIFO.c
	arm-none-eabi-gcc -gdwarf-2 "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/My_Own_RTOS/inc" -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/CMSIS_V5" -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/STM32_F103C6_Drivers/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"My_Own_RTOS/MYRTOS_FIFO.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
My_Own_RTOS/Scheduler.o: ../My_Own_RTOS/Scheduler.c
	arm-none-eabi-gcc -gdwarf-2 "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/My_Own_RTOS/inc" -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/CMSIS_V5" -I"D:/Mastering Embedded System/Unit15(RTOS)/My Own RTOS/STM32_F103C6_Drivers/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"My_Own_RTOS/Scheduler.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

