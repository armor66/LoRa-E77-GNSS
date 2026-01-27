#ifndef LRNS_HEADER
#define LRNS_HEADER

#include "main.h"

#define GPS_SPEED_THRS	(3)	//threshold value for GPS speed in km/h to show course

extern const double deg_to_rad;       //deg to rad multiplyer

//uint8_t *get_air_packet_tx(void);
//uint8_t *get_air_packet_rx(void);

void init_lrns(void);
void fill_air_packet(uint32_t current_uptime);
void parse_air_packet(uint32_t current_uptime);
void process_all_devices(void);
void calc_point_position(uint8_t another_device);

void calc_relative_position(uint8_t another_device);
//void calc_timeout(uint32_t current_uptime);
void calc_fence(void);
uint8_t check_any_alarm_fence_timeout(void);

void ublox_to_this_device(uint8_t device_number);
void rx_to_devices(uint8_t device_number);
void check_traced(uint8_t device_number);
void clear_fix_data(uint8_t device_number);
int8_t getADC_calibration(void);
void getADC_sensors(void);

//Struct with all devices info
struct devices_struct
{
	//COMMON
    uint8_t device_received;  				//a device id, single ASCII symbol
	uint8_t exist_flag;             //does a device exist?
	uint8_t beacon_lost;
	uint8_t beacon_flag;

	int8_t beacon_traced;
	int8_t index_in_flash;
	uint8_t flwtrek_flag;
	uint8_t fence_flag;				//is a predefined fence distance reached?

//	uint8_t emergency_flag;
//	uint8_t antitheft_flag;
//	uint8_t bcntohalt_flag;
	uint8_t beeper_flag;

    //TIME
//    uint32_t timeout;				//timeout in seconds since last activity (RX of coordinates)
    uint8_t timeout_flag;			//set when predefined timeout occurs

    uint8_t fix_type_opt;
    uint8_t valid_fix_flag;
    uint8_t valid_date_flag;
    uint16_t p_dop;

    uint8_t time_hours;
	uint8_t time_minutes;
	uint8_t time_seconds;

    //ABSOLUTE COORDINATES
    union
    {
    	int32_t as_integer;             //latitude in decimal degrees (-90...+90)
        uint8_t as_array[4];
    } latitude;

    union
    {
    	int32_t as_integer;             //longitude in decimal degrees (-180...+180)
        uint8_t as_array[4];
    } longitude;

	uint8_t gps_speed;
	int16_t gps_heading;

    //RELATIVE COORDINATES
    uint32_t distance;          //distance in meters to a device
    int16_t azimuth_deg_signed;       //heading to a device, degrees
    double azimuth_rad;			//heading to a device, radians

    uint8_t core_voltage;
    uint8_t batt_voltage;
    int8_t core_temperature;
    int8_t rssi;
    int8_t snr;
};
struct devices_struct **get_devices(void);

struct point_groups_struct
{
	int8_t index_in_flash;
	int8_t points_stored;
	int8_t points_in_ram;
};
struct point_groups_struct **get_point_groups(void);

#endif /*LRNS_HEADER*/
