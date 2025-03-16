#include <string.h>
#include "uart_it.h"
#include "main.h"
#include "lrns.h"
#include "gpio.h"
#include "settings.h"

#define PVTsize 100
#define UBX_HW_VER_SIZE 40				//for UBX_MON_VER: Header(2)+Class(1)+ID(1)+Length(2)+swVersion(30)+hwVersion(10)
#define UBX_CFG_SIZE 14					//answer for CFG-MSGOUT-UBX_NAVPVT_UART1, CFG-UART1OUTPROT-UBX(NMEA)
#define UBX_CFG_FLAG 10				//hwVersion(10)

uint8_t PVTbuffer[PVTsize] = {0};
uint8_t GNSSbuffer[UBX_HW_VER_SIZE] = {0};

void USART1_IRQHandler(void)
{
	struct settings_struct *p_settings;
	p_settings = get_settings();

    if (USART1->ISR & USART_ISR_RXNE_RXFNE)	//сброс аппаратный чтением регистра
    {
    	if(main_flags.GPSconfigureFlag || main_flags.GPScheckFlag)	//GPSconfigureFlag or GPScheckFlag has set
		{
			if(main_flags.uartIdx > UBX_HW_VER_SIZE) main_flags.uartIdx = 0;		//secure check
			GNSSbuffer[main_flags.uartIdx] = USART1->RDR;
			//if header is not valid
			if(main_flags.uartIdx == 1 && (GNSSbuffer[0] != 0xB5 || GNSSbuffer[1] != 0x62))	// || GNSSbuffer[2] != 0x0A || GNSSbuffer[3] != 0x04))
	   		{
	   			USART1->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
//	   			memset(GNSSbuffer, 0, UBX_HW_VER_SIZE);
	   			main_flags.GPScheckFlag = 0;	//after init_gnss one check only
	   		}
			//if it is answer for UBX-CFG-UART1OUT
			else if((main_flags.uartIdx == UBX_CFG_SIZE) && GNSSbuffer[2] == 0x06 && GNSSbuffer[3] == 0x8B && GNSSbuffer[4] == 0x09)
			{
				if(GNSSbuffer[UBX_CFG_FLAG] == 0x07)
				{
					main_flags.nav_pvt_ram_flag = GNSSbuffer[UBX_CFG_SIZE];
				}
				else if(GNSSbuffer[UBX_CFG_FLAG] == 0x01)
				{
					main_flags.out_ubx_ram_flag = GNSSbuffer[UBX_CFG_SIZE];
				}
				else if(GNSSbuffer[UBX_CFG_FLAG] == 0x02)
				{
					main_flags.out_nmea_ram_flag= GNSSbuffer[UBX_CFG_SIZE];
				}
	   			USART1->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
		   		memset(GNSSbuffer, 0, UBX_CFG_SIZE);
		   		main_flags.GPScheckFlag = 0;	//after init_gnss one check only
			}
			//if it is answer for UBX_HV_VER
	   		else if((main_flags.uartIdx == UBX_HW_VER_SIZE) && GNSSbuffer[2] == 0x0A && GNSSbuffer[3] == 0x04)
	   		{
	   			USART1->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
	   			(GNSSbuffer[UBX_HW_VER_SIZE-1] == 0x41)?
	   					(main_flags.ubx_hwVersion = (GNSSbuffer[UBX_HW_VER_SIZE-1] - 0x37)):
						(main_flags.ubx_hwVersion = (GNSSbuffer[UBX_HW_VER_SIZE-1] - 0x30));
	   			memset(GNSSbuffer, 0, UBX_HW_VER_SIZE);
	   		}
	   		else main_flags.uartIdx++;	//do not increment index in case 1 or 2
		}
    	else		//normal operation if(!GPSconfigureFlag && !GPScheckFlag)
	    {
	    	PVTbuffer[main_flags.uartIdx] = USART1->RDR;	   		//PVTbuffer[main_flags.uartIdx] = uartByte;
	   		if(main_flags.uartIdx == 3 && (PVTbuffer[0] != 0xB5 || PVTbuffer[1] != 0x62 || PVTbuffer[2] != 0x01 || PVTbuffer[3] != 0x07))
	   		{
	   			USART1->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;
	   			memset(PVTbuffer, 0, PVTsize);			//for(uint8_t i = 0; i < PVTsize; i++) { PVTbuffer[i] = 0; }
	   		}
	   		else if (main_flags.uartIdx >= 83)	//83 = 77(pDOP) + 6 (main_flags.uartIdx >= PVTsize)
	   		{
	   			USART1->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
	   			ublox_to_this_device(p_settings->device_number);
	   			led_green_off();			//led_green_on on PPS IRQ
	   		} else main_flags.uartIdx++;	//do not increment index in case 1 or 2
		}
	}
}
