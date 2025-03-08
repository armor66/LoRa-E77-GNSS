#include <string.h>
#include <math.h>
#include "lrns.h"
#include "settings.h"
//#include "radio.h"
#include "menu.h"
#include "gpio.h"
//#include "gnss.h"

const double rad_to_deg = 57.29577951308232;        //rad to deg multiplyer
const double deg_to_rad = 0.0174532925199433;       //deg to rad multiplyer

//Air packet structure and fields position
#define PACKET_NUM_ID_POS           (0)			//byte pos in a packet
#define BYTE_NUM_POS           		(5)			//bits pos in a byte
#define PACKET_NUM_MASK           	(0xE0)
#define PACKET_ID_MASK           	(0x1F)
#define PACKET_FLAGS_POS            (1)
#define PACKET_LATITUDE_POS         (2)
#define PACKET_LONGITUDE_POS        (6)
#define PACKET_ALTITUDE_POS        	(10)

#define AIR_PACKET_LEN   (0x0D)	//payload len only, no syncword/crc included   (FSK_PP7_PLOAD_LEN_12_BYTE)

struct settings_struct *p_settings_lrns;

struct devices_struct devices[DEVICES_ON_AIR_MAX + 1];        //structures array for devices from 1 to DEVICES_IN_GROUP. Index 0 is invalid and always empty.
struct devices_struct *p_devices[DEVICES_ON_AIR_MAX + 1];		//structure pointers array

struct devices_struct **get_devices(void)
{
	for (uint8_t dev = 0; dev <= DEVICES_ON_AIR_MAX; dev++)
	{
		p_devices[dev] = &devices[dev];
	}
	return &p_devices[0];
}

struct point_groups_struct point_groups[MEMORY_POINT_GROUPS];
struct point_groups_struct *p_point_groups[MEMORY_POINT_GROUPS];

struct point_groups_struct **get_point_groups(void)
{
	for (uint8_t grp = 0; grp < MEMORY_POINT_GROUPS; grp++)
	{
		p_point_groups[grp] = &point_groups[grp];
	}
	return &p_point_groups[0];
}

double distance[DEVICES_ON_AIR_MAX + 1];
double arc_length[DEVICES_ON_AIR_MAX + 1];
int16_t azimuth_deg_signed[DEVICES_ON_AIR_MAX + 1];
int16_t azimuth_deg_unsigned[DEVICES_ON_AIR_MAX + 1];
double azimuth_rad[DEVICES_ON_AIR_MAX + 1];

struct devices_struct **pp_devices;
struct point_groups_struct **pp_point_groups;
struct points_struct **pp_points_lrns;

void init_lrns(void)
{
	//Get external things
	p_settings_lrns = get_settings();
	pp_points_lrns = get_points();
//	pp_lost_device_lrns = get_lost_device();
//	pp_trekpoints_lrns = get_tekpoints();
	pp_devices = get_devices();
	pp_point_groups = get_point_groups();
	//Clear mem
    for (uint8_t dev = 1; dev <= p_settings_lrns->devices_on_air; dev++)		//DEVICES_ON_AIR_MAX
    {
        memset(&devices[dev], 0, sizeof(devices[dev]));
    }
//    for (uint8_t grp = 0; grp < MEMORY_POINT_GROUPS; grp++)		//DEVICES_ON_AIR_MAX
//    {
//        memset(&point_groups[grp], 0, sizeof(point_groups[grp]));
//    }
}

void ublox_to_this_device(uint8_t device_number)
{
	devices[device_number].fix_type_opt = PVTbuffer[20+6];				//all 6 types
	devices[device_number].valid_fix_flag = (PVTbuffer[21+6] & 0x01);	//bit0 only
	devices[device_number].valid_date_flag = (PVTbuffer[11+6] & 0x01);	//valid UTC Date

		devices[device_number].longitude.as_array[0] = PVTbuffer[30];
		devices[device_number].longitude.as_array[1] = PVTbuffer[31];
		devices[device_number].longitude.as_array[2] = PVTbuffer[32];
		devices[device_number].longitude.as_array[3] = PVTbuffer[33];

		devices[device_number].latitude.as_array[0] = PVTbuffer[34];
		devices[device_number].latitude.as_array[1] = PVTbuffer[35];
		devices[device_number].latitude.as_array[2] = PVTbuffer[36];
		devices[device_number].latitude.as_array[3] = PVTbuffer[37];

		devices[device_number].gps_speed = ((PVTbuffer[63+6]<<24)+(PVTbuffer[62+6]<<16)+(PVTbuffer[61+6]<<8)+PVTbuffer[60+6])/278 & 0xFF;		// 0 - 255 km/h
		devices[device_number].gps_heading = ((PVTbuffer[67+6]<<24)+(PVTbuffer[66+6]<<16)+(PVTbuffer[65+6]<<8)+PVTbuffer[64+6])/100000 & 0x1FF;	// 0 - 511 degrees
		devices[device_number].p_dop = (PVTbuffer[77+6]<<8)+PVTbuffer[76+6];

		if(main_flags.fix_valid >= p_settings_lrns->devices_on_air)		//at least 3 fix valid occurred
		{
			main_flags.fix_valid = p_settings_lrns->devices_on_air;
			if(!main_flags.first_time_locked)	//if not locked yet
			{
				//set CFG-TP-PERIOD_LOCK_TP1=3.000.000 to manage ADC and UART only ones on period
//				if(p_settings_lrns->spreading_factor == 12)
//				{
//					USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;
//					serialPrint(set_three_seconds, sizeof(set_three_seconds));
//				}
				main_flags.first_time_locked = 1;
				main_flags.short_beeps = 3;		//glad tidings
			}
		}else  if(devices[device_number].valid_fix_flag) main_flags.fix_valid++;
}
void rx_to_devices(uint8_t device_number)
{
//	uint8_t *buffer = bufNode[device_number];
	uint8_t *buffer = bufferRx;

	devices[device_number].beacon_flag = buffer[0] >> 7;
	if(((buffer[0] & 0x70) >> 4) == 5) devices[device_number].emergency_flag = 1;
	else if(((buffer[0] & 0x70) >> 4) == 2) devices[device_number].emergency_flag = 0;
//	devices[device_number].emergency_flag = (buffer[0] & 0x40) >> 6;
//	devices[device_number].alarm_flag = (buffer[0] & 0x20) >> 5;
//	devices[device_number].gather_flag = (buffer[0] & 0x10) >> 4;
	devices[device_number].beeper_flag = (buffer[0] & 0x8) >> 3;
//	main_flags.beeper_flag_received = (buffer[0] & 0x8) >> 3;
	devices[device_number].device_num = buffer[0] & 0x07;

//	devices[device_number].is_moving =
	devices[device_number].fix_type_opt = (buffer[1] & 0x60) >> 5;			//only 2 bits used to transmit
	devices[device_number].valid_fix_flag = ((buffer[1] & 0x10) >> 4);		//bit0 only
	devices[device_number].batt_voltage = (buffer[1] & 0x0F)+27;			//in decimal volts
	devices[device_number].p_dop = buffer[2];								//0...25.5

	devices[device_number].longitude.as_array[0] = buffer[3];
	devices[device_number].longitude.as_array[1] = buffer[4];
	devices[device_number].longitude.as_array[2] = buffer[5];
	devices[device_number].longitude.as_array[3] = buffer[6];

	devices[device_number].latitude.as_array[0] = buffer[7];
	devices[device_number].latitude.as_array[1] = buffer[8];
	devices[device_number].latitude.as_array[2] = buffer[9];
	devices[device_number].latitude.as_array[3] = buffer[10];

//	devices[device_number].time_hours = (buffer[11] >> 4) + 8;
//	devices[device_number].time_minutes = (((buffer[11] & 0xF) << 2) + ((buffer[12] & 0xC0) >> 6));
//	devices[device_number].time_seconds = (buffer[12] & 0x3F);

	devices[device_number].gps_speed = (buffer[11] >> 1);					// 0 - 128 km/h
	devices[device_number].gps_heading = (((buffer[11] & 0x1) << 8) + buffer[12]);

	devices[device_number].rssi = buffer[BUFFER_AIR_SIZE];
	devices[device_number].snr = buffer[BUFFER_AIR_SIZE + 1];

	if(pp_devices[p_settings_lrns->device_number]->valid_fix_flag)
	{							// ignore 7 point groups to get 7, 8, 9 ,10, 11 - 5 device groups
		int8_t group_start_index = (MEMORY_POINT_GROUPS + device_number - 1) * MEMORY_SUBPOINTS;

		if(buffer[1] & 0x80)		//round robin if remote device is moving
		{
			for(int8_t ind = 6; ind > 0; ind--)	//6->7, 5->6, 4->5, 3->4, 2->3, 1->2
			{
				pp_points_lrns[group_start_index + ind + 1]->exist_flag = pp_points_lrns[group_start_index + ind]->exist_flag;
				pp_points_lrns[group_start_index + ind + 1]->latitude.as_integer = pp_points_lrns[group_start_index + ind]->latitude.as_integer;
				pp_points_lrns[group_start_index + ind + 1]->longitude.as_integer = pp_points_lrns[group_start_index + ind]->longitude.as_integer;
			}
		}		//fill subpoint1 new data in any case
/*		pp_lost_device_lrns[device_number][0].exist_flag = 1;
		pp_lost_device_lrns[device_number][0].latitude.as_integer = devices[device_number].latitude.as_integer;
		pp_lost_device_lrns[device_number][0].longitude.as_integer = devices[device_number].longitude.as_integer;
*/
		pp_points_lrns[group_start_index + 1]->exist_flag = 1;
		pp_points_lrns[group_start_index + 1]->latitude.as_integer = devices[device_number].latitude.as_integer;
		pp_points_lrns[group_start_index + 1]->longitude.as_integer = devices[device_number].longitude.as_integer;

/*beacon_traced always zero if timeout_threshold=0*/
		devices[device_number].beacon_traced = p_settings_lrns->timeout_threshold / p_settings_lrns->devices_on_air;		//!validFixFlag[time_slot] delay
		if(devices[device_number].beacon_flag) devices[device_number].beacon_traced = 30 / p_settings_lrns->devices_on_air;	//always 30 seconds before save it
		devices[device_number].beacon_lost = 0;

		if(devices[device_number].emergency_flag)
		{	//Alarms group = 0, sub point 1
			pp_points_lrns[0 + device_number]->exist_flag = 1;	//3
			pp_points_lrns[0 + device_number]->latitude.as_integer = devices[device_number].latitude.as_integer;
			pp_points_lrns[0 + device_number]->longitude.as_integer = devices[device_number].longitude.as_integer;
			devices[device_number].beacon_traced = 30 / p_settings_lrns->devices_on_air;	//always 30 seconds before save it
			if(!main_flags.display_status) shortBeeps(device_number);				//emergency_flag received
		}
		if(devices[device_number].alarm_flag)
		{	//Alarms group = 0, sub point 1
			pp_points_lrns[0 + device_number]->exist_flag = 1;	//5
			pp_points_lrns[0 + device_number]->latitude.as_integer = devices[device_number].latitude.as_integer;
			pp_points_lrns[0 + device_number]->longitude.as_integer = devices[device_number].longitude.as_integer;
			devices[device_number].beacon_traced = 30 / p_settings_lrns->devices_on_air;	//always 30 seconds before save it
		}
		if(devices[device_number].gather_flag)
		{	//Alarms group = 0, sub point 1
			pp_points_lrns[0 + device_number]->exist_flag = 1;	//9
			pp_points_lrns[0 + device_number]->latitude.as_integer = devices[device_number].latitude.as_integer;
			pp_points_lrns[0 + device_number]->longitude.as_integer = devices[device_number].longitude.as_integer;
			devices[device_number].beacon_traced = 30 / p_settings_lrns->devices_on_air;	//always 30 seconds before save it
		}
	}

}

void check_traced(uint8_t device_number)
{
	if(--devices[device_number].beacon_traced <= 0)	//may be decremented below 0
	{
		devices[device_number].beacon_traced = 0;
		devices[device_number].beacon_lost = 1;
	}
}

void clear_fix_data(uint8_t device_number)
{
	devices[device_number].fix_type_opt = 0;			//only 2 bits used to transmit
	devices[device_number].valid_fix_flag = 0;			//bit0 only
	devices[device_number].p_dop = 0;
}

void calc_point_position(uint8_t point)		//MEMORY_POINTS_TOTAL = 8 * 28 = 224
{
//	pp_points_lrns = get_points();
	//valid GPS fix - pDop and accuracy
//	if(PVTbuffer[21+6] & 0x01)
//	{
		//my position
		double Latitude0 = ((double)(PVTbuffer[37]<<24)+(PVTbuffer[36]<<16)+(PVTbuffer[35]<<8)+PVTbuffer[34]) / 10000000 * deg_to_rad;
		double Longitude0 = ((double)(PVTbuffer[33]<<24)+(PVTbuffer[32]<<16)+(PVTbuffer[31]<<8)+PVTbuffer[30]) /10000000 * deg_to_rad;

		//position of the device to calculate relative position
		double Latitude1 = ((double)pp_points_lrns[point]->latitude.as_integer) / 10000000 * deg_to_rad;
		double Longitude1 = ((double)pp_points_lrns[point]->longitude.as_integer) / 10000000 * deg_to_rad;
//uint32_t distance;          //distance in meters to a device
		pp_points_lrns[point]->distance = (uint32_t)(6371008 * sqrt(pow((Latitude1 - Latitude0), 2) + pow((cos(Latitude0) * (Longitude1 - Longitude0)), 2)));

		double X = cos(Latitude1) * sin(Longitude1 - Longitude0);
		double Y = cos(Latitude0) * sin(Latitude1) - sin(Latitude0) * cos(Latitude1) * cos(Longitude1 - Longitude0);
		pp_points_lrns[point]->azimuth_rad = atan2(X,Y);
//int16_t azimuth_deg_signed;       //heading to a device, degrees
		pp_points_lrns[point]->azimuth_deg_signed = (int16_t)(pp_points_lrns[point]->azimuth_rad * rad_to_deg);		//convert to deg

	    if(Longitude1 < Longitude0) {pp_points_lrns[point]->azimuth_rad += 2*M_PI;}

	    if(pp_points_lrns[point]->distance == 0) pp_points_lrns[point]->azimuth_rad = 0;
//	}
}

void calc_relative_position(uint8_t another_device)
{
		//my position
		double Latitude0 = ((double)(PVTbuffer[37]<<24)+(PVTbuffer[36]<<16)+(PVTbuffer[35]<<8)+PVTbuffer[34]) / 10000000 * deg_to_rad;
		double Longitude0 = ((double)(PVTbuffer[33]<<24)+(PVTbuffer[32]<<16)+(PVTbuffer[31]<<8)+PVTbuffer[30]) /10000000 * deg_to_rad;

		//position of the device to calculate relative position
		double Latitude1 = ((double)(pp_devices[another_device]->latitude.as_integer)) / 10000000 * deg_to_rad;
		double Longitude1 = ((double)(pp_devices[another_device]->longitude.as_integer)) / 10000000 * deg_to_rad;

//		if (Latitude0 == Latitude1) Latitude1 += 0.00000001;       //slightly shift the position
//		if (Longitude0 == Longitude1) Longitude1 += 0.00000001;       //slightly shift the position
//формула учитывает изменение расстояния между меридианами в зависимости от широты в радианах
		distance[another_device] = 6371008 * sqrt(pow((Latitude1 - Latitude0), 2) + pow((cos(Latitude0) * (Longitude1 - Longitude0)), 2));
//		arc_length[another_device] = (twice_mean_earth_radius * asin( sqrt( pow(sin((Latitude1 - Latitude0) / 2), 2) +
//		                	cos(Latitude1) * cos(Latitude0) * pow(sin((Longitude1 - Longitude0) / 2), 2))));
		double X = cos(Latitude1) * sin(Longitude1 - Longitude0);
		double Y = cos(Latitude0) * sin(Latitude1) - sin(Latitude0) * cos(Latitude1) * cos(Longitude1 - Longitude0);
		azimuth_rad[another_device] = atan2(X,Y);
//		azimuth_rad[another_device] = (atan((Longitude1 - Longitude0) /
//		                    log(tan(pi_div_by_4 + Latitude1 / 2) / tan(pi_div_by_4 + Latitude0 / 2))));
		azimuth_deg_signed[another_device] = (int16_t)(azimuth_rad[another_device] * rad_to_deg);		//convert to deg
//	    if (Longitude1 > Longitude0)		 {;}
	    if(Longitude1 < Longitude0) {azimuth_rad[another_device] += 2*M_PI;}

	    if(distance[another_device] == 0) azimuth_rad[another_device] = 0;

	    azimuth_deg_unsigned[another_device] = (int16_t)(azimuth_rad[another_device] * rad_to_deg);		//convert to deg
}

void calc_fence(void)		//all devices should be processed before calling this func
{
	if (p_settings_lrns->fence_threshold != FENCE_ALARM_DISABLED)
	{
		for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < DEVICE_NUMBER_LAST + 1; dev++)		//devices only, not for mem point
		{
			if (devices[dev].exist_flag)
			{
				if (devices[dev].distance > p_settings_lrns->fence_threshold)
				{
					devices[dev].fence_flag = 1;
				}
				else
				{
					devices[dev].fence_flag = 0;
				}
			}
		}
	}
}

uint8_t check_any_alarm_fence_timeout(void)
{
	for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < MEMORY_POINT_LAST + 1; dev++)		//devices + mem points
	{
		if (devices[dev].exist_flag)
		{
			if ((devices[dev].alarm_flag) || (devices[dev].fence_flag) || (devices[dev].timeout_flag))
			{
				return 1;
			}
		}
	}

	return 0;
}

/**********************ADC conversion stuff******************/
uint16_t adc_buffer[3];

int8_t getADC_calibration(void)
{
	uint8_t calibration_index;
	uint16_t calibration_factor_accumulated = 0;

	// the ADC voltage regulator is enabled (ADVREGEN=1 and LDORDY=1)
	if(ADC->CR & ADC_CR_ADVREGEN)
	{
		ADC->CFGR1 &= ~(ADC_CFGR1_AUTOFF | ADC_CFGR1_DMACFG | ADC_CFGR1_DMAEN);
		ADC->CR	   &= ~(ADC_CR_ADSTP | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN);
		ADC->CR	|= ADC_CR_ADCAL;
	}else return -1;

    for (calibration_index = 0; calibration_index < 8; calibration_index++)
    {
    	/* Start ADC calibration */
    	ADC->CR |= ADC_CR_ADCAL;

    	/* Wait for calibration completion */
    	while (ADC->CR & ADC_CR_ADCAL);

    	calibration_factor_accumulated += (uint16_t)(ADC->CALFACT);
    }
    /* Compute average */
    calibration_factor_accumulated /= calibration_index;
    /* Apply calibration factor */
    ADC->CR |= ADC_CR_ADEN;
    ADC->CALFACT = calibration_factor_accumulated;

    /* Enable ADC AUTOFF and DMA mode */
    ADC->CFGR1 |= ADC_CFGR1_AUTOFF | ADC_CFGR1_DMAEN;

    /* Disable peripheral for the first DMA configuration*/
	DMA1_Channel1->CCR &= ~DMA_CCR_EN;

	return calibration_factor_accumulated;
}

void getADC_sensors(void)
{
//	HAL_ADC_Start_DMA(&hadc,(uint32_t*)adc_buffer, 3);
//  ADC->CFGR1 |= ADC_CFGR1_DMAEN; (enabled after calibration)

    /* Clear regular group conversion flag and overrun flag */
	ADC->ISR |= ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;

/****************HAL_DMA_Start_IT (to Start the DMA channel)*/
//	DMA1_Channel1->CCR &= ~DMA_CCR_EN; (peripheral already disabled in the handler)*/
    /* Configure the source, destination address and the data length & clear flags*/
		/* Clear all flags (if channel transfer error occurs)*/
		DMA1->IFCR = 0xFFFFFFFF;
		/* Configure DMA Channel data length */
		DMA1_Channel1->CNDTR = 3;
		/* Peripheral to Memory */
		DMA1_Channel1->CPAR = (uint32_t)&ADC->DR;
		DMA1_Channel1->CMAR = (uint32_t)&adc_buffer;

	/* Enable the transfer complete interrupt */
	DMA1_Channel1->CCR |= DMA_CCR_TCIE;
    /* Enable the Peripheral */
    DMA1_Channel1->CCR |= DMA_CCR_EN;
/***************HAL_DMA_Start_IT (DMA channel started)*/

    /* Start ADC group regular conversion */
	ADC->CR |= ADC_CR_ADSTART;
}

//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
void DMA1_Channel1_IRQHandler(void)
{
	if(DMA1->ISR & DMA_ISR_TCIF1)	//Transfer Complete Interrupt
	{
//		main_flags.adc_calibration_factor = ADC->CALFACT; check if lost when AUTOFF has set
		/* Disable peripheral to clear transfer complete interrupt here after*/
		DMA1_Channel1->CCR &= ~DMA_CCR_EN;
	    DMA1_Channel1->CCR &= ~DMA_CCR_TCIE;
	    /* Clear the transfer complete flag */
	    DMA1->IFCR = DMA_IFCR_CTCIF1;

		uint16_t coreVoltage = 0;//(VREFINT_CAL_VREF * 1510) / adc_buffer[2];
		int16_t temperatureDegreeC = 0;

	    if ((uint32_t)*VREFINT_CAL_ADDR != (uint32_t)0xFFFFU)
	    {
	      /* Device with Reference voltage calibrated in production: use device optimized parameters */
	    	coreVoltage = __LL_ADC_CALC_VREFANALOG_VOLTAGE(adc_buffer[2], ADC_RESOLUTION_12B);
	    }
	    else
	    {
	      /* Device with Reference voltage not calibrated in production: use generic parameters */
	    	coreVoltage = 0;//(VREFINT_CAL_VREF * 1510) / measuredLevel;
	    }
		devices[p_settings_lrns->device_number].core_voltage = (uint8_t)(coreVoltage /10 - 270);	//2700-:-4200 -> 0...150

		if (((int32_t)*TEMPSENSOR_CAL2_ADDR - (int32_t)*TEMPSENSOR_CAL1_ADDR) != 0)
		{
		    /* Device with temperature sensor calibrated in production: use device optimized parameters */
		    temperatureDegreeC = __LL_ADC_CALC_TEMPERATURE(coreVoltage,	adc_buffer[1], LL_ADC_RESOLUTION_12B);
		}
		else
		{
		    /* Device with temperature sensor not calibrated in production: use generic parameters */
		    temperatureDegreeC = 0;/*__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(TEMPSENSOR_TYP_AVGSLOPE, TEMPSENSOR_TYP_CAL1_V,
		    								TEMPSENSOR_CAL1_TEMP, coreVoltage, adc_buffer[1], LL_ADC_RESOLUTION_12B);*/
		}
		devices[p_settings_lrns->device_number].core_temperature = (int8_t)((temperatureDegreeC << 8) / 256);	//from int16 to q8.7

		uint32_t battVoltage = adc_buffer[0] * coreVoltage /2053;									//~4096/2
		devices[p_settings_lrns->device_number].batt_voltage = (uint8_t)(battVoltage /10 - 270);	//2700-:-4200 -> 0...150
	}
}
