#include "main.h"
#include "gpio.h"
#include "timer_it.h"
#include "tim.h"
#include "buttons.h"
#include "bit_band.h"
#include "menu.h"
#include "settings.h"
#include "lrns.h"
#include "e77radio.h"
#include "radio.h"
//#include "gnss.h"		//for trek points
//#include "compass.h"

int8_t pattern_index = 0;
int8_t long_beep_ones = 0;

uint8_t *p_tx_power_values_tim;

struct settings_struct *p_settings_tim;
struct devices_struct **pp_devices_tim;

void timer_it_init(void)
{
/*URS: Update request source
 * 0: Any of the following events generate an update interrupt or DMA request if enabled:
 * – Counter overflow/underflow
 * – Setting the UG bit
 * 1: Only counter overflow/underflow generates an update interrupt or DMA request if enabled:
 * no need to clear interrupt when CNT is reinitialized by software using the UG bit in TIMx_EGR register
*/
	TIM1->CR1 |= TIM_CR1_URS;
	TIM2->CR1 |= TIM_CR1_URS;
//	TIM16->CR1 |= TIM_CR1_URS;
//	TIM17->CR1 |= TIM_CR1_URS;

	p_tx_power_values_tim = get_tx_power_values();
	p_settings_tim = get_settings();
	pp_devices_tim = get_devices();
}
const uint8_t timeslot_pattern[2][103] =
//	| PVT|Slot1 OnRxDone time <642mS		+358mS|Slot2 OnTxDone time <627mS		  | PVT|Slot3					   | draw menu|
//	|----|-------------------------|---------|----|-------------------------|---------|----|-------------------------|---------|--
//	0 50 100mS	                             1000 1100mS							  2000 2100								 3000
  {{0,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,5,4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,5,//4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,6 };
//	| PVT|Slot3					   | draw menu|PVT|Slot4					| draw menu|PVT|Slot5					 | draw menu|
//	|----|-------------------------|---------|----|-------------------------|---------|----|-------------------------|---------|--
//	2000 2100mS								 3000 3100mS							  4000 4100mS							   5000mS
	4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,5,4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,5,4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,5,4,1 },
//	SF = 12, CR3(4/7), Packet duration in air = 1122.3
// PPS	 |Slot1 		AIR time 1122mS		PPS 		| draw menu|  |Slot2 		 PPS   		AIR time 1122mS      | draw menu|
//	|----|-----------------------------------|----|-----|-------------|---------------|----|-------------------------|---------|
//	0 50 100mS	                             1000		1250mS						  2000				    	  	2750	   3000
   {0,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0, 0,0,0,3,4,5,4,5,4,1,2,0,0,0,0,0,0,0,0,0, 0,0,0,0,6,0,0,0,0,0,0,0,0,3,4,5,4,5,4,1 }};
//	|----|---------------------------|------------------|-------------|----------------------------|-----------------|---------|
//	0 50 100mS	                    +700     		   +1150mS	     1600				  	      +700				+1150	   3000

static void restartPattern(void)
{
	getADC_sensors();
	clear_fix_data(p_settings_tim->device_number);
	timer1_stop();
	timer1_start();
	main_flags.time_slot = 0;			// from TIM1_IRQ case: 2
	main_flags.time_slot_timer_ovf = 0;
	main_flags.pps_synced = 0;
	led_blue_on();			//led_blue_off on case 1 (50ms)
	led_red_on();			//as pattern_started on PPS
	if(((p_settings_tim->spreading_factor == 12) && (p_settings_tim->device_number > 2)) || !main_flags.nav_pvt_ram_flag)
	{
		shortBeeps(2);
	}
}

void TIM1_UP_IRQHandler(void)
{
	TIM1->SR &= ~TIM_SR_UIF;                    //clear interrupt
	if(!main_flags.scanRadioFlag)
    {
		main_flags.time_slot_timer_ovf++;
		(p_settings_tim->spreading_factor == 12)? (pattern_index = 1): (pattern_index = 0);
		switch (timeslot_pattern[pattern_index][main_flags.time_slot_timer_ovf])
		{
		case 0:			//do nothing
			break;

		case 1:			//50mS
			if(p_settings_tim->spreading_factor == 12) main_flags.time_slot++;
			if(p_settings_tim->devices_on_air == main_flags.time_slot)	//if PPS did not come, but cycle completed + 50mS
			{
				restartPattern();
				break;
			}
			if(p_settings_tim->spreading_factor != 12) main_flags.time_slot++;
//clear what should be received or not in this slot after draw menu has finished
			if(p_settings_tim->device_number != main_flags.time_slot)
			{
				clear_fix_data(main_flags.time_slot);

				if(!main_flags.antitheft_flag_confurmed) main_flags.antitheft_flag_received = 0;
				if(!main_flags.bcntohalt_flag_confurmed) main_flags.bcntohalt_flag_received = 0;
			}
			if(p_settings_tim->spreading_factor == 12)
			{	//set TX iq_inversion = 0 so that module №3 can receive data
				if(p_settings_tim->device_number == main_flags.time_slot)	//transmit LORA_IQ_NORMAL
				{
					Radio.SetTxConfig(MODEM_LORA, p_tx_power_values_tim[p_settings_tim->tx_power_opt], 0,
					LORA_BANDWIDTH,	p_settings_tim->spreading_factor, p_settings_tim->coding_rate_opt,
					LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, true, 0, 0, LORA_IQ_NORMAL, TX_TIMEOUT_VALUE);
				}//set RX iq_inversion = 1 to not receive from other beacon but receive from module №3
				if(p_settings_tim->device_number != main_flags.time_slot)	//receive LORA_IQ_INVERTED, BUFFER_AIR_SIZE = 3
				{
					Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, p_settings_tim->spreading_factor,
					p_settings_tim->coding_rate_opt, 0, LORA_PREAMBLE_LENGTH, LORA_SYMBOL_TIMEOUT,
					LORA_FIX_LENGTH_PAYLOAD_ON,	3, true, 0, 0, LORA_IQ_INVERTED, false);			//CRC, Hop, HopPer, IQ, single mode
				}
			}//end of spreading_factor == 12
			if(main_flags.pps_synced) led_green_on();	//to shorten it, instead of PPS start
			led_blue_off();				//led_blue_on on case 6 or PPS IRQ
			led_red_off();				//after new pattern started
			break;

		case 2:			//100mS
			led_green_off();
//			if(main_flags.long_beeps)
//			{
//				main_flags.long_beeps_flag = 1;
//				led_w_on();
//			}

			if(main_flags.time_slot == p_settings_tim->device_number)	//this device:
			{	//if this device doesn't get gnss fix via uart 100mS after PPS, delay for full pattern time
				if(!pp_devices_tim[p_settings_tim->device_number]->valid_fix_flag)	main_flags.fix_valid--;
				if(main_flags.fix_valid > 0)	//do not transmit if no GNSS FIX
				{
					if(main_flags.display_status)
					{
						main_flags.endRX_2_TX = (uint16_t)TIM17->CNT;		//save interval from RX-end to TX-start
						timer17_stop();
					}
					main_flags.permit_actions = 0;
					led_red_on();					//start transmitting, end by OnTxDone
					set_transmit_data();			//State = TX_START;
				}else main_flags.fix_valid = 0;		//just to avoid negative values
			}
			else//if(time_slot != p_settings_tim->device_number)	other devices:
			{















				Radio.RxBoosted(700);			//start to receive SF=12 or not

//manage (pp_devices_phy[time_slot]->beeper_flag) on it own slot only, if beeper_flag received previously in this slot)
				if(pp_devices_tim[main_flags.time_slot]->beeper_flag)
				{
					led_w_on();
					pp_devices_tim[main_flags.time_slot]->beeper_flag = 0;
					long_beep_ones = 1;	//set here to finish on case 3 after starts here only
				}
			}
			break;

		case 3:				//check if remote devices on the air, draw menu items
			led_red_off();	//650ms after OnRxDone



















/*******************IF THERE IS ANY OF BEEP FLAGS****************************************/
//			(p_settings_tim->timeout_threshold || p_settings_tim->fence_threshold)? mute_off(): mute_on();
			if(main_flags.short_beeps && !main_flags.short_beeps_flag)	// && !main_flags.long_beeps)
			{
				main_flags.short_beeps_flag = 1;
				led_w_on();
			}
			if(long_beep_ones)
			{
				led_w_off();
				long_beep_ones = 0;
			}
//			if(main_flags.long_beeps && main_flags.long_beeps_flag)
//			{
//				main_flags.long_beeps--;
//				led_w_off();
//			}







/***********************************************************/
			main_flags.short_beeps? led_w_on(): (main_flags.update_screen = 1);
			main_flags.permit_actions = 1;		//process buttons here after
			break;

		case 4:
			if(main_flags.short_beeps_flag && main_flags.short_beeps)
			{
				led_w_off();
				main_flags.short_beeps--;
			}
			break;

		case 5:
			if(main_flags.short_beeps && main_flags.short_beeps_flag)	//(main_flags.short_beeps < 3))
			{
				led_w_on();
			}
			break;

		case 6:	//for SF=12 on the alien slot only (device 3 choose in witch slot to transmit)
			if(pp_devices_tim[main_flags.time_slot]->beeper_flag)
			{
				led_w_on();		//500mS to case 3
				pp_devices_tim[main_flags.time_slot]->beeper_flag = 0;
				long_beep_ones = 1;
			}
			break;
		default:
			break;
		}
    }
	else timer1_scanRadio_handle();	// if(scanRadioFlag)
}

void TIM2_IRQHandler(void)					//Scan buttons interval	void TIM3_IRQHandler(void)
{
	TIM2->SR &= ~TIM_SR_UIF;                    //clear interrupt
//	led_red_on();
	if (main_flags.buttons_scanned == 0)	//if not scanned yet
	{
		main_flags.button_code = scan_button(main_flags.processing_button);
		if (main_flags.button_code)	//perhaps get rid off the buttons_scanned and clear button_code in main.c/while (1)
		{
			main_flags.buttons_scanned = 1;
		}
	}
}

//void TIM16_IRQHandler(void)
//{
//	TIM16->SR &= ~TIM_SR_UIF;                    //clear interrupt
//	timer16_stop();
//	main_flags.current_point_group = 0;
//}


