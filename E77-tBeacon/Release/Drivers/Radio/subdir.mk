################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Radio/radio.c \
../Drivers/Radio/radio_driver.c \
../Drivers/Radio/stm32_timer.c 

OBJS += \
./Drivers/Radio/radio.o \
./Drivers/Radio/radio_driver.o \
./Drivers/Radio/stm32_timer.o 

C_DEPS += \
./Drivers/Radio/radio.d \
./Drivers/Radio/radio_driver.d \
./Drivers/Radio/stm32_timer.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Radio/%.o Drivers/Radio/%.su Drivers/Radio/%.cyclo: ../Drivers/Radio/%.c Drivers/Radio/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../Core/Inc -I../Drivers/BSP/STM32WLxx_Nucleo -I../Drivers/CMSIS/Include -I"/home/z/STM32Cube/NewModule/E77-tBeacon/Drivers/Radio" -I"/home/z/STM32Cube/NewModule/E77-tBeacon/Drivers/STM32WLxx_HAL_Driver/Inc" -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Radio

clean-Drivers-2f-Radio:
	-$(RM) ./Drivers/Radio/radio.cyclo ./Drivers/Radio/radio.d ./Drivers/Radio/radio.o ./Drivers/Radio/radio.su ./Drivers/Radio/radio_driver.cyclo ./Drivers/Radio/radio_driver.d ./Drivers/Radio/radio_driver.o ./Drivers/Radio/radio_driver.su ./Drivers/Radio/stm32_timer.cyclo ./Drivers/Radio/stm32_timer.d ./Drivers/Radio/stm32_timer.o ./Drivers/Radio/stm32_timer.su

.PHONY: clean-Drivers-2f-Radio

