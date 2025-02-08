#include "main.h"
#include "gpio.h"
#include "exti_it.h"
#include "buttons.h"
#include "tim.h"
#include "lrns.h"
#include "settings.h"

void EXTI9_5_IRQHandler(void)	//(BTN_3_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF5;		//clear interrupt
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
void EXTI0_IRQHandler(void)		//(BTN_1_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF0;		//clear interrupt
	disable_buttons_interrupts();
	main_flags.pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
	main_flags.processing_button = BUTTON_PWR;
	timer2_start();
}
void EXTI4_IRQHandler(void)		//(PPS_Pin)
{
	EXTI->PR1 = EXTI_PR1_PIF4;		//clear interrupt
	main_flags.pps_synced = 1;

	struct settings_struct *p_settings;
	struct devices_struct **pp_devices;
	p_settings = get_settings();
	pp_devices = get_devices();

	//start timer ones of 3, 4 or 5 seconds
	if(PVTbuffer[16]%p_settings->devices_on_air == 0)// || (p_settings_phy->spreading_factor == 12))
	{
		timer1_stop();
		timer1_start();						// start time slot timer right after PPS

		main_flags.time_slot = 0;			// from TIM1_IRQ case: 2
		main_flags.time_slot_timer_ovf = 0;

//		if(p_settings->spreading_factor == 12) led_blue_on();

//		main_flags.pattern_started = 1;
		led_red_on();
	}

	if(p_settings->device_number == (main_flags.time_slot + 1)) clear_fix_data(main_flags.time_slot + 1);	//before uart handling finished
	if(p_settings->spreading_factor == 12 && p_settings->device_number == 2) clear_fix_data(2);	//for beacon №2 only to transmit
//	if(p_settings->spreading_factor != 12) led_blue_on();	//PPS received
	led_blue_on();

	led_w_off();								//avoid extra beeps
//	main_flags.short_beeps = 0;
	getADC_sensors();	//lrns.c

	if(main_flags.pps_counter++ > 60)			//if 60sec no buttons activity
	{
		main_flags.pps_counter = 60;
		lcd_off();
		main_flags.display_status = 0;
//for TPS7330 Vthresold=2.64V, for TPS7333 Vthresold=2.87V(287-270=17), 0==270(2.70V) (actually ~2.95V)
		if(pp_devices[p_settings->device_number]->batt_voltage < 30)
		{
			longBeepsBlocking(1);				//long beep to prevent silent "RESET"
			HAL_Delay(50);
			release_power();
		}
	}

	main_flags.uartIdx = 0;
	USART2->RDR;				//!!!очистка регистра чтением!!!иначе прерывание сработает сразу
	USART2->CR1 = 0x00000000;	//иначе работает только после resetа
	USART2->CR1 = USART_CR1_UE;
	USART2->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE;	//1: USART interrupt generated whenever ORE = 1 or RXFNE = 1 in the USART_ISR register

}
