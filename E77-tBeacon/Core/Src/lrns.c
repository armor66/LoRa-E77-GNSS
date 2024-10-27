#include <string.h>
#include <math.h>
#include "config.h"
#include "lrns.h"
#include "settings.h"
#include "radio.h"
#include "menu.h"
#include "gpio.h"
#include "adc_if.h"
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
//1 byte (0) device number and ID (single char)
//1 byte (1) flags
//4 bytes (2, 3, 4, 5) lat
//4 bytes (6, 7, 8, 9) lon
//2 bytes (10, 11) altitude
//TOTAL 12 bytes - see radio.c for AIR_PACKET_LEN

#define AIR_PACKET_LEN   (0x0D)	//payload len only, no syncword/crc included   (FSK_PP7_PLOAD_LEN_12_BYTE)

struct settings_struct *p_settings_lrns;
uint8_t *p_air_packet_tx;
uint8_t *p_air_packet_rx;

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

//uint8_t air_packet_tx[AIR_PACKET_LEN];
//uint8_t air_packet_rx[AIR_PACKET_LEN];
//
//uint8_t *get_air_packet_tx(void) {return &air_packet_tx[0];}
//uint8_t *get_air_packet_rx(void) {return &air_packet_rx[0];}

double distance[DEVICES_ON_AIR_MAX + 1];
double arc_length[DEVICES_ON_AIR_MAX + 1];
int16_t azimuth_deg_signed[DEVICES_ON_AIR_MAX + 1];
int16_t azimuth_deg_unsigned[DEVICES_ON_AIR_MAX + 1];
double azimuth_rad[DEVICES_ON_AIR_MAX + 1];
struct devices_struct **pp_devices;
struct points_struct **pp_points_lrns;
//struct trekpoints_struct **pp_trekpoints_lrns;

/*struct gps_num_struct gps_num;
struct gps_num_struct *get_gps_num(void)
{
	return &gps_num;
}
*/
void init_lrns(void)
{
	//Get external things
	p_settings_lrns = get_settings();
	pp_points_lrns = get_points();
//	pp_trekpoints_lrns = get_tekpoints();
	pp_devices = get_devices();
//	p_air_packet_tx = get_air_packet_tx();
//	p_air_packet_rx = get_air_packet_rx();
	//Clear mem
    for (uint8_t dev = 1; dev <= p_settings_lrns->devices_on_air; dev++)		//DEVICES_ON_AIR_MAX
    {
        memset(&devices[dev], 0, sizeof(devices[dev]));
    }

	//This device number
//	this_device = p_settings_lrns->device_number;		//not used

    //Activate this device
//	devices[this_device].exist_flag = 1;				//not used
//	devices[this_device].device_id = p_settings_lrns->device_id;

//instead of memset (str83-86)
//	for(int8_t i = 0; i < (p_settings_lrns->devices_on_air + 1); i++)
//   {
//	   devices[i].beacon_traced = 0;
//	   devices[i].beacon_lost = 0;
//   }
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

		devices[device_number].gps_speed = ((PVTbuffer[63+6]<<24)+(PVTbuffer[62+6]<<16)+(PVTbuffer[61+6]<<8)+PVTbuffer[60+6])/278 & 0xFF;			// 0 - 255 km/h
		devices[device_number].gps_heading = ((PVTbuffer[67+6]<<24)+(PVTbuffer[66+6]<<16)+(PVTbuffer[65+6]<<8)+PVTbuffer[64+6])/100000 & 0x1FF;	// 0 - 511 degrees
		devices[device_number].p_dop = (PVTbuffer[77+6]<<8)+PVTbuffer[76+6];

		if(devices[device_number].valid_fix_flag) main_flags.fix_valid = p_settings_lrns->devices_on_air;	//3, 4 or 5
}
void rx_to_devices(uint8_t device_number)
{
//	uint8_t *buffer = bufNode[device_number];
	uint8_t *buffer = bufferRx;

	devices[device_number].beacon_flag = buffer[0] >> 7;
	devices[device_number].emergency_flag = (buffer[0] & 0x40) >> 6;
	devices[device_number].alarm_flag = (buffer[0] & 0x20) >> 5;
	devices[device_number].gather_flag = (buffer[0] & 0x10) >> 4;
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
		if(bufferRx[3] & 0x01)		//round robin if remote device is moving
		{
			for(int8_t ind = 6; ind > 0; ind--)	//6->7, 5->6, 4->5, 3->4, 2->3, 1->2
			{
				pp_points_lrns[group_start_index + ind + 1]->exist_flag = pp_points_lrns[group_start_index + ind]->exist_flag;
				pp_points_lrns[group_start_index + ind + 1]->latitude.as_integer = pp_points_lrns[group_start_index + ind]->latitude.as_integer;
				pp_points_lrns[group_start_index + ind + 1]->longitude.as_integer = pp_points_lrns[group_start_index + ind]->longitude.as_integer;
			}
		}		//fill subpoint1 new data in any case
		pp_points_lrns[group_start_index + 1]->exist_flag = 1;
		pp_points_lrns[group_start_index + 1]->latitude.as_integer = devices[device_number].latitude.as_integer;
		pp_points_lrns[group_start_index + 1]->longitude.as_integer = devices[device_number].longitude.as_integer;
//beacon_traced always zero if timeout_threshold=0
		devices[device_number].beacon_traced = p_settings_lrns->timeout_threshold / p_settings_lrns->devices_on_air;		//!validFixFlag[time_slot] delay
		if(devices[device_number].beacon_flag) devices[device_number].beacon_traced = 30 / p_settings_lrns->devices_on_air;	//always 30 seconds before save it
		devices[device_number].beacon_lost = 0;

		if(devices[device_number].emergency_flag)
		{	//Alarms group = 0, sub point 1
			pp_points_lrns[0 + device_number]->exist_flag = 1;	//3
			pp_points_lrns[0 + device_number]->latitude.as_integer = devices[device_number].latitude.as_integer;
			pp_points_lrns[0 + device_number]->longitude.as_integer = devices[device_number].longitude.as_integer;
			devices[device_number].beacon_traced = 30 / p_settings_lrns->devices_on_air;	//always 30 seconds before save it
			if(!pp_devices[p_settings_lrns->device_number]->display_status) shortBeeps(device_number);				//emergency_flag received
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
		devices[device_number].beacon_lost = 1;
	}
}
void clear_fix_data(uint8_t device_number)
{
	devices[device_number].fix_type_opt = 0;			//only 2 bits used to transmit
//	if(devices[device_number].valid_fix_flag > 0) devices[device_number].valid_fix_flag--;
	devices[device_number].valid_fix_flag = 0;			//bit0 only
	devices[device_number].p_dop = 0;
}

//pp_devices_lrns[another_device]->latitude.as_integer
//pp_devices_lrns[another_device]->longitude.as_integer
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
//	pp_devices = get_devices();							//uint8_t *buffer = bufNode[another_device];
//	//valid GPS fix - pDop and accuracy
//	if(PVTbuffer[21+6] & 0x01)	//&& (pp_devices[another_device]->valid_fix_flag)) has checked previously in "OnRxDone() -> rx_to_devices(time_slot)"
//	{
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
//	}
}

void calc_timeout(uint32_t current_uptime)
{
	for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < DEVICE_NUMBER_LAST + 1; dev++)	//calculated even for this device and used to alarm about own timeout upon lost of PPS signal
	{
		if (devices[dev].exist_flag == 1)
		{
			devices[dev].timeout = current_uptime - devices[dev].timestamp; //calc timeout for each active device

        	if (p_settings_lrns->timeout_threshold != TIMEOUT_ALARM_DISABLED) //if enabled
        	{
				if (devices[dev].timeout > p_settings_lrns->timeout_threshold)
				{
					if (dev == p_settings_lrns->device_number)
					{
						if (get_abs_pps_cntr() <= PPS_SKIP)	//if this is a timeout right after power up, ignore timeout alarm, do not set the flag
						{									//feature, not a bug: once first PPS appeared, a short beep occurs (do "<= PPS_SKIP" to disable this, #TBD)
							devices[dev].timeout_flag = 0;
						}
						else
						{
							devices[dev].timeout_flag = 1;
						}
					}
					else
					{
						devices[dev].timeout_flag = 1; //set flag for alarm
					}
				}
				else
				{
					devices[dev].timeout_flag = 0;
				}
        	}
        }
    }
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

//void toggle_emergency(void) {
//	(devices[this_device].emergency_flag == 0)? (devices[this_device].emergency_flag = 1): (devices[this_device].emergency_flag = 0);
//}
//void toggle_alarm(void) {
//	(devices[this_device].alarm_flag == 0)? (devices[this_device].alarm_flag = 1): (devices[this_device].alarm_flag = 0);
//}
//void toggle_gather(void) {
//	(devices[this_device].gather_flag == 0)? (devices[this_device].gather_flag = 1): (devices[this_device].gather_flag = 0);
//}
//void toggle_trigger(void) {
//	(devices[this_device].trigger_flag == 0)? (devices[this_device].trigger_flag = 1): (devices[this_device].trigger_flag = 0);
//}
//void toggle_rxOnly(void) {
//	(devices[this_device].rxOnly_flag == 0)? (devices[this_device].rxOnly_flag = 1): (devices[this_device].rxOnly_flag = 0);
//}
//
//uint8_t get_my_alarm_status(void)
//{
//	return devices[this_device].alarm_flag;
//}

void getADC_sensors(uint8_t device)
{
	uint16_t coreVoltage = (uint16_t)SYS_GetBatteryLevel();
	uint32_t battVoltage = GetChannelLevel() * coreVoltage /2053;		//~4096/2

	devices[device].core_voltage = (uint8_t)(coreVoltage /10 - 270);	//2700-:-4200 -> 0...150	(coreVoltage - 2700) / 10
	devices[device].batt_voltage = (uint8_t)(battVoltage /10 - 270);	//2700-:-4200 -> 0...150	(battVoltage - 2700) / 10

	devices[device].core_temperature = (int8_t)(SYS_GetTemperatureLevel() /256);
}
