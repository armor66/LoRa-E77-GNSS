################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ST7735.c \
../Core/Src/adc.c \
../Core/Src/adc_if.c \
../Core/Src/bno055.c \
../Core/Src/buttons.c \
../Core/Src/compass.c \
../Core/Src/eeprom.c \
../Core/Src/fonts.c \
../Core/Src/gnss.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/iwdg.c \
../Core/Src/lptim.c \
../Core/Src/lrns.c \
../Core/Src/main.c \
../Core/Src/menu.c \
../Core/Src/rtc.c \
../Core/Src/settings.c \
../Core/Src/spi.c \
../Core/Src/stm32_adv_trace_if.c \
../Core/Src/stm32_lpm_if.c \
../Core/Src/stm32wlxx_hal_msp.c \
../Core/Src/stm32wlxx_it.c \
../Core/Src/subghz.c \
../Core/Src/sys_app.c \
../Core/Src/sys_debug.c \
../Core/Src/system_stm32wlxx.c \
../Core/Src/tim.c \
../Core/Src/timer_if.c \
../Core/Src/usart.c \
../Core/Src/usart_if.c 

OBJS += \
./Core/Src/ST7735.o \
./Core/Src/adc.o \
./Core/Src/adc_if.o \
./Core/Src/bno055.o \
./Core/Src/buttons.o \
./Core/Src/compass.o \
./Core/Src/eeprom.o \
./Core/Src/fonts.o \
./Core/Src/gnss.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/iwdg.o \
./Core/Src/lptim.o \
./Core/Src/lrns.o \
./Core/Src/main.o \
./Core/Src/menu.o \
./Core/Src/rtc.o \
./Core/Src/settings.o \
./Core/Src/spi.o \
./Core/Src/stm32_adv_trace_if.o \
./Core/Src/stm32_lpm_if.o \
./Core/Src/stm32wlxx_hal_msp.o \
./Core/Src/stm32wlxx_it.o \
./Core/Src/subghz.o \
./Core/Src/sys_app.o \
./Core/Src/sys_debug.o \
./Core/Src/system_stm32wlxx.o \
./Core/Src/tim.o \
./Core/Src/timer_if.o \
./Core/Src/usart.o \
./Core/Src/usart_if.o 

C_DEPS += \
./Core/Src/ST7735.d \
./Core/Src/adc.d \
./Core/Src/adc_if.d \
./Core/Src/bno055.d \
./Core/Src/buttons.d \
./Core/Src/compass.d \
./Core/Src/eeprom.d \
./Core/Src/fonts.d \
./Core/Src/gnss.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/iwdg.d \
./Core/Src/lptim.d \
./Core/Src/lrns.d \
./Core/Src/main.d \
./Core/Src/menu.d \
./Core/Src/rtc.d \
./Core/Src/settings.d \
./Core/Src/spi.d \
./Core/Src/stm32_adv_trace_if.d \
./Core/Src/stm32_lpm_if.d \
./Core/Src/stm32wlxx_hal_msp.d \
./Core/Src/stm32wlxx_it.d \
./Core/Src/subghz.d \
./Core/Src/sys_app.d \
./Core/Src/sys_debug.d \
./Core/Src/system_stm32wlxx.d \
./Core/Src/tim.d \
./Core/Src/timer_if.d \
./Core/Src/usart.d \
./Core/Src/usart_if.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../Core/Inc -I../Drivers/BSP/STM32WLxx_Nucleo -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -I../Utilities/trace/adv_trace -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/timer -I../Utilities/lpm/tiny_lpm -I../Middlewares/Third_Party/SubGHz_Phy -I../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../SubGHz_Phy/App -I../SubGHz_Phy/Target -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ST7735.cyclo ./Core/Src/ST7735.d ./Core/Src/ST7735.o ./Core/Src/ST7735.su ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/adc_if.cyclo ./Core/Src/adc_if.d ./Core/Src/adc_if.o ./Core/Src/adc_if.su ./Core/Src/bno055.cyclo ./Core/Src/bno055.d ./Core/Src/bno055.o ./Core/Src/bno055.su ./Core/Src/buttons.cyclo ./Core/Src/buttons.d ./Core/Src/buttons.o ./Core/Src/buttons.su ./Core/Src/compass.cyclo ./Core/Src/compass.d ./Core/Src/compass.o ./Core/Src/compass.su ./Core/Src/eeprom.cyclo ./Core/Src/eeprom.d ./Core/Src/eeprom.o ./Core/Src/eeprom.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/gnss.cyclo ./Core/Src/gnss.d ./Core/Src/gnss.o ./Core/Src/gnss.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/iwdg.cyclo ./Core/Src/iwdg.d ./Core/Src/iwdg.o ./Core/Src/iwdg.su ./Core/Src/lptim.cyclo ./Core/Src/lptim.d ./Core/Src/lptim.o ./Core/Src/lptim.su ./Core/Src/lrns.cyclo ./Core/Src/lrns.d ./Core/Src/lrns.o ./Core/Src/lrns.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/menu.cyclo ./Core/Src/menu.d ./Core/Src/menu.o ./Core/Src/menu.su ./Core/Src/rtc.cyclo ./Core/Src/rtc.d ./Core/Src/rtc.o ./Core/Src/rtc.su ./Core/Src/settings.cyclo ./Core/Src/settings.d ./Core/Src/settings.o ./Core/Src/settings.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32_adv_trace_if.cyclo ./Core/Src/stm32_adv_trace_if.d ./Core/Src/stm32_adv_trace_if.o ./Core/Src/stm32_adv_trace_if.su ./Core/Src/stm32_lpm_if.cyclo ./Core/Src/stm32_lpm_if.d ./Core/Src/stm32_lpm_if.o ./Core/Src/stm32_lpm_if.su ./Core/Src/stm32wlxx_hal_msp.cyclo ./Core/Src/stm32wlxx_hal_msp.d ./Core/Src/stm32wlxx_hal_msp.o ./Core/Src/stm32wlxx_hal_msp.su ./Core/Src/stm32wlxx_it.cyclo ./Core/Src/stm32wlxx_it.d ./Core/Src/stm32wlxx_it.o ./Core/Src/stm32wlxx_it.su ./Core/Src/subghz.cyclo ./Core/Src/subghz.d ./Core/Src/subghz.o ./Core/Src/subghz.su ./Core/Src/sys_app.cyclo ./Core/Src/sys_app.d ./Core/Src/sys_app.o ./Core/Src/sys_app.su ./Core/Src/sys_debug.cyclo ./Core/Src/sys_debug.d ./Core/Src/sys_debug.o ./Core/Src/sys_debug.su ./Core/Src/system_stm32wlxx.cyclo ./Core/Src/system_stm32wlxx.d ./Core/Src/system_stm32wlxx.o ./Core/Src/system_stm32wlxx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/timer_if.cyclo ./Core/Src/timer_if.d ./Core/Src/timer_if.o ./Core/Src/timer_if.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su ./Core/Src/usart_if.cyclo ./Core/Src/usart_if.d ./Core/Src/usart_if.o ./Core/Src/usart_if.su

.PHONY: clean-Core-2f-Src

