#include "main.h"
#include "gpio.h"
#include "exti_it.h"
#include "buttons.h"
#include "tim.h"
#include "lptim.h"
#include "lrns.h"
#include "settings.h"

__inline static void manage_uart(void)
{
	main_flags.uartIdx = 0;
	USART2->RDR;				//!!!очистка регистра чтением!!!иначе прерывание сработает сразу
	USART2->CR1 = 0x00000000;	//иначе работает только после resetа
	USART2->CR1 = USART_CR1_UE;
	USART2->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE;	//1: USART interrupt generated whenever ORE = 1 or RXFNE = 1 in the USART_ISR register
}

void EXTI0_IRQHandler(void)	//(BTN_3_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF0;		//clear interrupt
	disable_buttons_interrupts();
	main_flags.pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
	main_flags.processing_button = BUTTON_DOWN_ESC;
	timer2_start();
}

void EXTI1_IRQHandler(void)		//(BTN_2_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF1;		//clear interrupt
	disable_buttons_interrupts();
	main_flags.pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
	main_flags.processing_button = BUTTON_UP_OK;
	timer2_start();
}
void EXTI4_IRQHandler(void)		//(BTN_1_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF4;		//clear interrupt
	disable_buttons_interrupts();
	main_flags.pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
	main_flags.processing_button = BUTTON_PWR;
	timer2_start();
}
void EXTI9_5_IRQHandler(void)		//(PPS_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF8;		//clear interrupt
	main_flags.pps_synced = 1;

	struct settings_struct *p_settings;
	struct devices_struct **pp_devices;
	p_settings = get_settings();
	pp_devices = get_devices();

	//start time slot timer right after PPS ones per 3, 4 or 5 seconds
	if(PVTbuffer[16]%p_settings->devices_on_air == 0)
	{
		timer1_stop();
		timer1_start();

		main_flags.time_slot = 0;
		main_flags.time_slot_timer_ovf = 0;

		if(p_settings->spreading_factor == 12)
		{
			led_blue_on();
			getADC_sensors();	//lrns.c
			//ones per period for this device(3) only before uart handling
			clear_fix_data(p_settings->device_number);
//			if(main_flags.fix_valid == 3) manage_uart();	//manage ADC and UART only ones per period
		}
//		main_flags.pattern_started = 1;
		led_red_on();
	}

	if(p_settings->spreading_factor != 12)
	{
		led_blue_on();
		getADC_sensors();	//lrns.c
		//on this device time slot only before uart handling
		if(p_settings->device_number == (main_flags.time_slot + 1)) clear_fix_data(main_flags.time_slot + 1);
//		manage_uart();		//manage ADC and UART on each PPS
	}
//	else
//	{
//		if(main_flags.fix_valid < 3) manage_uart();		//manage ADC and UART on each PPS
//	}
	manage_uart();		//manage ADC and UART on each PPS

	if(main_flags.short_beeps_flag)		//avoid extra beeps
	{
		main_flags.short_beeps_flag = 0;
		main_flags.short_beeps = 0;
		led_w_off();
	}

	if(main_flags.pps_counter++ > 60)	//if 60sec no buttons activity
	{
		main_flags.pps_counter = 60;
		lptim1_stop();					//lcd_off();
		main_flags.display_status = 0;	//do not draw or change menu
//for TPS7330 Vthresold=2.64V, for TPS7333 Vthresold=2.87V(287-270=17), 0==270(2.70V) (actually ~2.95V)
		if(pp_devices[p_settings->device_number]->batt_voltage < 30)	//270 + 30 (<3.00V)
		{
			led_w_on();		//long beep to prevent silent shutdown
			HAL_Delay(750);
			release_power();
		}
	}
}

