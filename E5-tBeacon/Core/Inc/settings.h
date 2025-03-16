#include "main.h"

#ifndef SETTINGS_HEADER
#define SETTINGS_HEADER

//#define BEACON
#ifndef BEACON
#define IS_BEACON		(0)			//false
#else
#define IS_BEACON		(1)			//true
#endif

#define TIMEOUT_ALARM_DISABLED		(0)
#define FENCE_ALARM_DISABLED		(0)

#define DEVICE_NUMBER_FIRST (1)
#define DEVICES_ON_AIR_MAX	(5)
#define DEVICE_NUMBER_LAST  (DEVICES_ON_AIR_MAX)

#define MEMORY_POINT_GROUPS	(7)
#define BEACON_POINT_GROUPS	(5)
#define MEMORY_SUBPOINTS	(8)	//(28)		//subpoints + 0
#define MEMORY_POINTS_TOTAL  (96)	//(MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS) * MEMORY_SUBPOINTS)

#define MEMORY_POINT_FIRST	(1)
#define MEMORY_POINT_LAST	(MEMORY_POINTS_TOTAL - 1)

//CODING_RATE SETTINGS	/* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define CODING_RATE_1_SETTING   	(1)
#define CODING_RATE_2_SETTING   	(2)
#define CODING_RATE_3_SETTING   	(3)
#define CODING_RATE_4_SETTING	  	(4)

#define CODING_RATE_FIRST_OPTION 		(CODING_RATE_1_SETTING)
#define CODING_RATE_LAST_OPTION 		(CODING_RATE_4_SETTING)

//POWER SETTINGS
#define TX_POWER_1MILLIW_SETTING   	(0)
#define TX_POWER_3MILLIW_SETTING   	(1)
#define TX_POWER_10MILLIW_SETTING   (2)
#define TX_POWER_31MILLIW_SETTING  	(3)
#define TX_POWER_100MILLIW_SETTING  (4)
#define TX_POWER_158MILLIW_SETTING  (5)

#define TX_POWER_FIRST_OPTION 		(TX_POWER_1MILLIW_SETTING)
#define TX_POWER_LAST_OPTION 		(TX_POWER_158MILLIW_SETTING)

//FREQ
#define FREQ_CHANNEL_FIRST  (3)
#define FREQ_CHANNEL_LAST   (68)

//TIMEOUT
#define TIMEOUT_THRESHOLD_MIN 	(0)
#define TIMEOUT_THRESHOLD_MAX 	(240)
#define TIMEOUT_THRESHOLD_STEP 	(30)

//FENCE
#define FENCE_THRESHOLD_MIN 	(0)
#define FENCE_THRESHOLD_MAX 	(250)
#define FENCE_THRESHOLD_STEP 	(50)

//Structure with settings
struct settings_struct
{
	uint8_t settings_init_flag;
    uint8_t device_number;              //this device number in group, 1...DEVICES_IN_GROUP
    uint8_t devices_on_air;				//total number of devices on air, 1...DEVICES_IN_GROUP
    uint8_t spreading_factor;			//SPREADING_FACTOR   /* [SF7..SF12] */
    uint8_t coding_rate_opt;   			//CODINGRATE    /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
    uint8_t freq_channel;               //frequency tx/rx channel, LPD #1-69
    uint8_t tx_power_opt;               //tx power option, not an actual value
    uint8_t timeout_threshold;        	//timeout treshold in seconds, unsigned. if it == 0, then timeout alarm not trigger (but, anyway, timeout is counting). See TIMEOUT_ALARM_DISABLED
    uint8_t fence_threshold;        	//fence treshold in meters, unsigned. if it == 0, then fence alarm not trigger. See FENCE_ALARM_DISABLED
    int8_t time_zone_hour;				//can be 0 ... 14 if time_zone_dir = 1; and 0 ... 12 if time_zone_dir = -1

    union {
    	int16_t as_integer;
    	uint8_t as_array[2];
    } accel_offset_x;

    union {
    	int16_t as_integer;
    	uint8_t as_array[2];
    } accel_offset_y;

    union {
    	int16_t as_integer;
    	uint8_t as_array[2];
    } accel_offset_z;

    union {
    	int16_t as_integer;         		//magnetometer offset X for hard iron compensation
    	uint8_t as_array[2];
    } magn_offset_x;

    union {
    	int16_t as_integer;         		//magnetometer offset Y for hard iron compensation
    	uint8_t as_array[2];
    } magn_offset_y;

    union {
    	int16_t as_integer;         		//magnetometer offset Y for hard iron compensation
    	uint8_t as_array[2];
    } magn_offset_z;

    union {
    	int16_t as_integer;
    	uint8_t as_array[2];
    } gyro_offset_x;

    union {
    	int16_t as_integer;
    	uint8_t as_array[2];
    } gyro_offset_y;

    union {
    	int16_t as_integer;
    	uint8_t as_array[2];
    } gyro_offset_z;

	union {
    	int16_t as_integer;
    	uint8_t as_array[2];
	} accel_radius;

	union {
		int16_t as_integer;
		uint8_t as_array[2];
	} magn_radius;
};

struct settings_struct *get_settings(void);
uint8_t *get_coding_rate_values(void);
uint8_t *get_tx_power_values(void);

void settings_load(void);
void settings_save(struct settings_struct *settings);
void settings_save_default(struct settings_struct *settings);

#ifndef BEACON
void saved_group_load(uint8_t group);
void points_group_save(uint8_t group);
void lost_device_load(uint8_t device);
void lost_device_save(uint8_t device);
void erase_point_groups(void);
void erase_saved_devices(void);

//Struct with all points info and his device "0"
struct points_struct
{
	uint8_t exist_flag;             //does a device exist?

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

    //RELATIVE COORDINATES
    uint32_t distance;          //distance in meters to a device
    int16_t azimuth_deg_signed;       //heading to a device, degrees
    double azimuth_rad;			//heading to a device, radians
};
//Struct with all devices info
struct points_struct **get_points(void);

void save_one_point(int8_t point_absolute_index);
void clear_points_group(int8_t group);

char *get_points_group_name(uint8_t group_number);
char *get_points_group_short(uint8_t group_number);
#endif
#endif /*SETTINGS_HEADER*/
