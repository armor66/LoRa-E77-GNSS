################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32wle5ccux.s 

OBJS += \
./Core/Startup/startup_stm32wle5ccux.o 

S_DEPS += \
./Core/Startup/startup_stm32wle5ccux.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -c -I"/home/z/STM32Cube/NewModule/E77-128x160/Drivers/BSP/STM32WLxx_Nucleo" -I"/home/z/STM32Cube/NewModule/E77-128x160/Drivers/Radio" -I"/home/z/STM32Cube/NewModule/E77-128x160/Drivers/STM32WLxx_HAL_Driver/Inc" -I"/home/z/STM32Cube/NewModule/E77-128x160/Drivers/CMSIS/Device/ST/STM32WLxx/Include" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32wle5ccux.d ./Core/Startup/startup_stm32wle5ccux.o

.PHONY: clean-Core-2f-Startup

