################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include STM32_F103C6_Drivers/RCC/subdir.mk
-include STM32_F103C6_Drivers/GPIO/subdir.mk
-include STM32_F103C6_Drivers/EXTI/subdir.mk
-include My_Own_RTOS/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Create_My_Own_RTOS.axf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Create_My_Own_RTOS.list \

OBJCOPY_BIN += \
Create_My_Own_RTOS.bin \


# All Target
all: Create_My_Own_RTOS.axf secondary-outputs

# Tool invocations
Create_My_Own_RTOS.axf: $(OBJS) $(USER_OBJS) D:\Mastering\ Embedded\ System\Unit15(RTOS)\My\ Own\ RTOS\STM32F103C8TX_FLASH.ld
	arm-none-eabi-gcc -gdwarf-2 -o "Create_My_Own_RTOS.axf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"D:\Mastering Embedded System\Unit15(RTOS)\My Own RTOS\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Create_My_Own_RTOS.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Create_My_Own_RTOS.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Create_My_Own_RTOS.list"
	@echo 'Finished building: $@'
	@echo ' '

Create_My_Own_RTOS.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Create_My_Own_RTOS.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
