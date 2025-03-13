
#include <stdio.h>			//for 'sprintf'#include "radio.h"
#include <stdlib.h>
#include <string.h>
#include "e77radio.h"
#include "radio.h"
#include "main.h"
#include "settings.h"
#include "lrns.h"
#include "gpio.h"
#include "spi.h"
#include "tim.h"
//#include "ST7735.h"
#include "lcd_display.h"

static RadioEvents_t RadioEvents;
//static States_t State = RX_START;

uint8_t bufferTx[BUFFER_AIR_SIZE];
uint8_t bufferRx[BUFFER_RX];

uint8_t *p_tx_power_values_rf;

int8_t rssi_by_channel[2][FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST + 1];
int8_t channel_ind = 0;

static void scanChannels(void);
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
static void OnTxDone(void);
static void OnRxError(void);
//static void OnRxTimeout(void);

struct settings_struct *p_settings_rf;
struct devices_struct **pp_devices_rf;

void radio_init(void)
{
	p_tx_power_values_rf = get_tx_power_values();
	p_settings_rf = get_settings();
	pp_devices_rf = get_devices();

	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxError = OnRxError;
//	RadioEvents.RxTimeout = OnRxTimeout;

	Radio.Init(&RadioEvents);		//Initializes driver callback functions: *TxDone *TxTimeout *RxDone *RxTimeout *FhssChangeChannel
	srand(Radio.Random());			//sets the radio in LoRa modem mode and disables all interrupts

    Radio.SetTxConfig(MODEM_LORA, p_tx_power_values_rf[p_settings_rf->tx_power_opt], 0, LORA_BANDWIDTH,			//MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
    		  p_settings_rf->spreading_factor, p_settings_rf->coding_rate_opt,				//LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_NORMAL, TX_TIMEOUT_VALUE);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, p_settings_rf->spreading_factor,		//MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
    		  p_settings_rf->coding_rate_opt, 0, LORA_PREAMBLE_LENGTH,
                        LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
						BUFFER_AIR_SIZE, true, 0, 0, LORA_IQ_NORMAL, true);				//0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_AIR_SIZE);

    Radio.SetChannel((433000 + 50 + p_settings_rf->freq_channel * 25) * 1000);	//(RF_FREQUENCY);

    HAL_Delay(Radio.GetWakeupTime());	// + TCXO_WORKAROUND_TIME_MARGIN);

	if(main_flags.scanRadioFlag) scanChannels();
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
	led_red_on();
	if(main_flags.display_status) timer17_start();

	memcpy(bufferRx, payload, BUFFER_AIR_SIZE);		//BufferAirSize
	bufferRx[BUFFER_AIR_SIZE] = (int8_t)rssi;		//BufferAirSize + 1
	bufferRx[BUFFER_AIR_SIZE + 1] = LoraSnr_FskCfo;	//BufferAirSize + 2
	//if received time slot == device number and it fix is valid
	if(((bufferRx[0] & 0x07) == main_flags.time_slot) && ((bufferRx[1] & 0x10) >> 4))
	{
		rx_to_devices(main_flags.time_slot); //where (uint8_t *buffer = bufferRx;)
	}
	//host device valid GPS fix - pDop and accuracy
	if(main_flags.fix_valid && !main_flags.rx_crc_error) calc_relative_position(main_flags.time_slot);
//	if(pp_devices_rf[p_settings_rf->device_number]->valid_fix_flag)	calc_relative_position(main_flags.time_slot);

	memset(bufferRx, 0, BUFFER_RX);

//	State = RX_DONE;
}

static void OnRxError(void)
{
	main_flags.rx_crc_error = 1;	//reset on case 2:100mS
}

//static void OnRxTimeout(void)
//{
//	State = RX_TO;
//}

static void OnTxDone(void)
{
	main_flags.permit_actions = 1;
	led_red_off();
}

void set_transmit_data(void)
{
	int8_t emergency_to_transmit;
	int8_t beeper_flag_to_transmit;



	(pp_devices_rf[p_settings_rf->device_number]->emergency_flag)? (emergency_to_transmit = 5): (emergency_to_transmit = 2);
	(pp_devices_rf[p_settings_rf->device_number]->beeper_flag)? (beeper_flag_to_transmit = 1): (beeper_flag_to_transmit = 0);












   	  bufferTx[0] =	(IS_BEACON << 7) + (emergency_to_transmit << 4) +
//    		  (pp_devices_rf[p_settings_rf->device_number]->emergency_flag << 6) +
//			  (pp_devices_rf[p_settings_rf->device_number]->alarm_flag << 5) +
//			  (pp_devices_rf[p_settings_rf->device_number]->gather_flag << 4) +
			  (beeper_flag_to_transmit << 3) + p_settings_rf->device_number;

   	  (pp_devices_rf[p_settings_rf->device_number]->gps_speed > GPS_SPEED_THRS)? (bufferTx[1] |= 1 << 7): (bufferTx[1] &= ~(1 << 7));	//if device is moving
   	  bufferTx[1] = ((PVTbuffer[20+6] & 0x03) << 5) +				//fix type, 2 bits only to transmit	//mask 0b0000 0011
   			  	  	 ((PVTbuffer[21+6] & 0x01) << 4) +				//fix valid, bit0 only				//mask 0b0000 0001
   					 (pp_devices_rf[p_settings_rf->device_number]->batt_voltage/10 & 0x0F);			//mask 0b0000 1111 (0-:150 -> 0-:-15)s
   	  bufferTx[2] = (pp_devices_rf[p_settings_rf->device_number]->p_dop/10 & 0xFF);					//pDop/10 (0...25.5)

	  bufferTx[3] = PVTbuffer[30];	//(uint8_t) ((UBLOX_Handle.lon >> 24) & 0xFF);		//*** longitude
	  bufferTx[4] = PVTbuffer[31];	//(uint8_t) ((UBLOX_Handle.lon >> 16) & 0xFF);
	  bufferTx[5] = PVTbuffer[32];	//(uint8_t) ((UBLOX_Handle.lon >> 8) & 0xFF);
	  bufferTx[6] = PVTbuffer[33];	//(uint8_t) ((UBLOX_Handle.lon >> 0) & 0xFF);

	  bufferTx[7] = PVTbuffer[34];	//(uint8_t) ((UBLOX_Handle.lat >> 24) & 0xFF);		//*** latitude
	  bufferTx[8] = PVTbuffer[35];	//(uint8_t) ((UBLOX_Handle.lat >> 16) & 0xFF);
	  bufferTx[9] = PVTbuffer[36];	//(uint8_t) ((UBLOX_Handle.lat >> 8) & 0xFF);
	  bufferTx[10] = PVTbuffer[37];	//(uint8_t) ((UBLOX_Handle.lat >> 0) & 0xFF);

//	  daylight_hour = PVTbuffer[14] + p_settings_phy->time_zone_hour;
//	  if(daylight_hour > 24) daylight_hour = daylight_hour - 24;
//	  (daylight_hour > 8)? (daylight_hour = daylight_hour - 8): (daylight_hour = 0);	//9-:_23
//	  bufferTx[11] = (daylight_hour << 4) +				//часы	  mask 0b00011111	  0-:-15
//	  	   	  	  	 ((PVTbuffer[15] & 0x3F) >> 2);		//минуты  mask 0b00111111
//	  bufferTx[12] = ((PVTbuffer[15] & 0x03) << 6) +	//минуты  mask 0b00000011
//			  	  	  (PVTbuffer[16] & 0x3F);			//секунды mask 0b00111111

	if(pp_devices_rf[p_settings_rf->device_number]->gps_speed > GPS_SPEED_THRS)
	{
		bufferTx[11] = ((pp_devices_rf[p_settings_rf->device_number]->gps_speed & 0x7F) << 1) +	// 0 - 128 km/h
					((pp_devices_rf[p_settings_rf->device_number]->gps_heading & 0x1FF) >> 8);	//mask 0b0000 0001 1111 1111
	  	bufferTx[12] = (pp_devices_rf[p_settings_rf->device_number]->gps_heading & 0xFF);			//mask 0b0000 0000 1111 1111
	}
	else bufferTx[12] = bufferTx[11] = 0;

	  Radio.Send(bufferTx, BUFFER_AIR_SIZE); 	// to be filled by attendee BufferAirSize
}

void timer1_scanRadio_handle(void)
{
	  led_toggle();	//led_red
	  Radio.SetChannel((433000 + 50 + ((channel_ind+1)*5 + FREQ_CHANNEL_FIRST) * 25) * 1000);	//(RF_FREQUENCY);
	  Radio.Rx(45);
	  rssi_by_channel[1][channel_ind] = Radio.Rssi(MODEM_LORA);
	  sprintf(&string_buffer[channel_ind + 1][0], "Ch %02d RSSI %04ddBm", (channel_ind*5 + FREQ_CHANNEL_FIRST), Radio.Rssi(MODEM_LORA));
	  channel_ind++;
	  if(((FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST)/5 - 1) < channel_ind) timer1_stop();	// 12 < channel_ind
}

void scanChannels(void)
{
	fill_screen(BLACK);
//	lcd_on
	while (1)//(GPIOA->IDR & BTN_2_Pin)		//wait for OK click to start cal
	{
		draw_str_by_rows(3, 33, "     TO SCAN", &Font_7x9, YELLOW,BLACK);
		draw_str_by_rows(3, 44, "    FREQUENCY", &Font_7x9, YELLOW,BLACK);
		draw_str_by_rows(0, 55, "     CHANNELS", &Font_7x9, YELLOW,BLACK);
		draw_str_by_rows(0, 77, "  Click ESC/DOWN", &Font_7x9, GREEN,BLACK);
		draw_str_by_rows(0, 99, "        OR", &Font_7x9, YELLOW,BLACK);
		draw_str_by_rows(3, 121, " POWER FOR REBOOT", &Font_7x9, GREEN,BLACK);

		while (GPIOA->IDR & BTN_1_Pin)
		{
			if (!(GPIOA->IDR & BTN_3_Pin))	//ECS for scan
			{
				for(int8_t i = 0; i < ((FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST)/5); i++)		// j < 13
				{
					rssi_by_channel[0][i] = -127;
				}
				fill_screen(BLACK);
//				sprintf(&string_buffers[0][0], "pressPWR to reboot");
//				sprintf(&string_buffers[1][0], "pressESC to rescan");
				for(int8_t i = 0; i < 7; i++)
				{
					channel_ind = 0;
					timer1_start();
					HAL_Delay(999);
//					timer1_stop();
					sprintf(&string_buffer[0][0], "channels = %02d", channel_ind);
					draw_str_by_rows(0, 4+0*11, &string_buffer[0][0], &Font_7x9, CYAN,BLACK);

					for (uint8_t j = 0; j < ((FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST)/5); j++)		// j < 13
					{
						if(rssi_by_channel[0][j] < rssi_by_channel[1][j]) rssi_by_channel[0][j] = rssi_by_channel[1][j];
						sprintf(&string_buffer[j + 1][0], "Ch %02d RSSI %04ddBm", (j*5 + FREQ_CHANNEL_FIRST), rssi_by_channel[0][j]);

						if(rssi_by_channel[0][j] > -60) draw_str_by_rows(0, 4+(j+1)*11, &string_buffer[j+1][0], &Font_7x9, RED,BLACK);
						else if(rssi_by_channel[0][j] < -80) draw_str_by_rows(0, 4+(j+1)*11, &string_buffer[j+1][0], &Font_7x9, GREEN,BLACK);
						else draw_str_by_rows(0, 4+(j+1)*11, &string_buffer[j+1][0], &Font_7x9, YELLOW,BLACK);
					}
				}
			}
		}
		NVIC_SystemReset();
	}
}
