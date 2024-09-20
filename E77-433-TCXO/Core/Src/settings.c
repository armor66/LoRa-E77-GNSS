#include "config.h"
#include "settings.h"
#include "lrns.h"
#include "flash_if.h"

#define FREQ_REGION_RU_VALUE		(64)
#define FREQ_REGION_IN_VALUE		(65)
#define FREQ_REGION_EU_VALUE		(68)
#define FREQ_REGION_US_VALUE		(115)
#define FREQ_REGION_KR_VALUE		(120)
#define FREQ_REGION_AS_VALUE		(123)

char *region[] = {"RU", "EU", "US"};

#define FREQ_REGION_VALUES_ARRAY 	{ 	FREQ_REGION_RU_VALUE, \
										FREQ_REGION_IN_VALUE, \
										FREQ_REGION_EU_VALUE,	\
										FREQ_REGION_US_VALUE, \
										FREQ_REGION_KR_VALUE, \
										FREQ_REGION_AS_VALUE}
/* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define CODING_RATE_1_VALUE   	(5)
#define CODING_RATE_2_VALUE   	(6)
#define CODING_RATE_3_VALUE   	(7)
#define CODING_RATE_4_VALUE  	(8)

#define CODING_RATE_VALUES_ARRAY 	{ 	0, 	\
										CODING_RATE_1_VALUE, 	\
										CODING_RATE_2_VALUE, 	\
										CODING_RATE_3_VALUE, 	\
										CODING_RATE_4_VALUE	}

#define TX_POWER_1MILLIW_VALUE   	(0)
#define TX_POWER_3MILLIW_VALUE   	(5)
#define TX_POWER_10MILLIW_VALUE   	(10)
#define TX_POWER_31MILLIW_VALUE  	(15)
#define TX_POWER_100MILLIW_VALUE  	(20)
#define TX_POWER_158MILLIW_VALUE  	(22)

#define TX_POWER_VALUES_ARRAY 		{ 	TX_POWER_1MILLIW_VALUE, 	\
										TX_POWER_3MILLIW_VALUE, 	\
										TX_POWER_10MILLIW_VALUE, 	\
										TX_POWER_31MILLIW_VALUE, 	\
										TX_POWER_100MILLIW_VALUE, 	\
										TX_POWER_158MILLIW_VALUE	}

//positions:
#define SETTINGS_INIT_FLAG_POS          	(0)
#define SETTINGS_DEVICE_NUMBER_POS      	(1)
#define SETTINGS_DEVICES_ON_AIR_POS			(2)
#define SETTINGS_SPREADING_FACTOR_POS      	(3)
//#define SETTINGS_FREQ_REGION_POS	        (4)
#define SETTINGS_CODING_RATE_POS     	  	(4)
#define SETTINGS_FREQ_CHANNEL_POS       	(5)
#define SETTINGS_TX_POWER_POS           	(6)
#define SETTINGS_TIMEOUT_THRESHOLD_POS   	(7)
#define SETTINGS_FENCE_THRESHOLD_POS   		(8)
#define SETTINGS_TIME_ZONE_HOUR_POS			(9)		//#define SETTINGS_TIME_ZONE_DIR_POS

#define SETTINGS_ACC_OFFSET_X_POS			(10)
#define SETTINGS_ACC_OFFSET_Y_POS			(12)
#define SETTINGS_ACC_OFFSET_Z_POS			(14)

#define SETTINGS_MAG_OFFSET_X_POS			(16)
#define SETTINGS_MAG_OFFSET_Y_POS			(18)
#define SETTINGS_MAG_OFFSET_Z_POS			(20)

#define SETTINGS_GYR_OFFSET_X_POS			(22)
#define SETTINGS_GYR_OFFSET_Y_POS			(24)
#define SETTINGS_GYR_OFFSET_Z_POS			(26)

#define SETTINGS_ACC_RADIUS_POS				(28)

#define SETTINGS_MAG_RADIUS_POS				(30)

/***********#define SETTINGS_POS			(32)***************/
//#define SETTINGS_ACCELEROMETER_POS			(10)	//TIME_ZONE_MINUTE
//#define SETTINGS_MAGN_OFFSET_X_POS			(12)
//#define SETTINGS_MAGN_OFFSET_Y_POS			(14)
//#define SETTINGS_MAGN_SCALE_X_POS			(16)
//#define SETTINGS_MAGN_SCALE_Y_POS			(20)
//#define SETTINGS_POS			(24)

//default values:
#define SETTINGS_INIT_FLAG_DEFAULT      	(0xAA)
#define SETTINGS_DEVICE_NUMBER_DEFAULT  	(1)
#define SETTINGS_DEVICES_ON_AIR_DEFAULT		(3)		//DEVICE_NUMBER_LAST)
#define SETTINGS_SPREADING_FACTOR_DEFAULT	(11)
//#define SETTINGS_FREQ_REGION_DEFAULT		(FREQ_REGION_FIRST_OPTION)
#define SETTINGS_CODING_RATE_DEFAULT		(CODING_RATE_2_SETTING)		// 2
#define SETTINGS_FREQ_CHANNEL_DEFAULT   	(FREQ_CHANNEL_FIRST)        //base freq is 433.050 and freq step is 25kHz, so CH0 - 433.050 (not valid, not used); CH1 - 433.075 (first LPD channel)
#define SETTINGS_TX_POWER_DEFAULT       	(TX_POWER_10MILLIW_SETTING)	// 2
#define SETTINGS_TIMEOUT_THRESHOLD_DEFAULT  (60)
#define SETTINGS_FENCE_THRESHOLD_DEFAULT  	(100)
#define SETTINGS_TIME_ZONE_HOUR_DEFAULT		(5)		//#define SETTINGS_TIME_ZONE_DIR_DEFAULT

#define SETTINGS_ACC_OFFSET_X_LSB_DEFAULT		(0x00)
#define SETTINGS_ACC_OFFSET_X_MSB_DEFAULT		(0x00)
#define SETTINGS_ACC_OFFSET_Y_LSB_DEFAULT		(0x00)
#define SETTINGS_ACC_OFFSET_Y_MSB_DEFAULT		(0x00)
#define SETTINGS_ACC_OFFSET_Z_LSB_DEFAULT		(0x00)
#define SETTINGS_ACC_OFFSET_Z_MSB_DEFAULT		(0x00)

#define SETTINGS_MAG_OFFSET_X_LSB_DEFAULT		(0x00)
#define SETTINGS_MAG_OFFSET_X_MSB_DEFAULT		(0x00)
#define SETTINGS_MAG_OFFSET_Y_LSB_DEFAULT		(0x00)
#define SETTINGS_MAG_OFFSET_Y_MSB_DEFAULT		(0x00)
#define SETTINGS_MAG_OFFSET_Z_LSB_DEFAULT		(0x00)
#define SETTINGS_MAG_OFFSET_Z_MSB_DEFAULT		(0x00)

#define SETTINGS_GYR_OFFSET_X_LSB_DEFAULT		(0x00)
#define SETTINGS_GYR_OFFSET_X_MSB_DEFAULT		(0x00)
#define SETTINGS_GYR_OFFSET_Y_LSB_DEFAULT		(0x00)
#define SETTINGS_GYR_OFFSET_Y_MSB_DEFAULT		(0x00)
#define SETTINGS_GYR_OFFSET_Z_LSB_DEFAULT		(0x00)
#define SETTINGS_GYR_OFFSET_Z_MSB_DEFAULT		(0x00)

#define SETTINGS_ACC_RADIUS_LSB_DEFAULT			(0x00)
#define SETTINGS_ACC_RADIUS_MSB_DEFAULT			(0x00)

#define SETTINGS_MAG_RADIUS_LSB_DEFAULT			(0x00)
#define SETTINGS_MAG_RADIUS_MSB_DEFAULT			(0x00)

//#define SETTINGS_ACCELEROMETER_DEFAULT		(0x03)		//707(0x0303)//TIME_ZONE_MINUTE
//#define SETTINGS_MAGN_OFFSET_X_DEFAULT		(0)
//#define SETTINGS_MAGN_OFFSET_Y_DEFAULT		(0)
//#define SETTINGS_MAGN_SCALE_XM0_DEFAULT		(0x3f)	//float 1.0 0x3f80
//#define SETTINGS_MAGN_SCALE_XM1_DEFAULT		(0x80)
//#define SETTINGS_MAGN_SCALE_XL0_DEFAULT		(0x00)
//#define SETTINGS_MAGN_SCALE_XL1_DEFAULT		(0x00)
//#define SETTINGS_MAGN_SCALE_YM0_DEFAULT		(0x3f)	//float 1.0 0x3f80
//#define SETTINGS_MAGN_SCALE_YM1_DEFAULT		(0x80)
//#define SETTINGS_MAGN_SCALE_YL0_DEFAULT		(0x00)
//#define SETTINGS_MAGN_SCALE_YL1_DEFAULT		(0x00)
//settings size
#define SETTINGS_SIZE						(32) //half-words (% sizeof(uint64_t)) == 0)

#define SETTINGS_PAGE		0x0803D800		//0x3D800 / 0x800 = page 123 (0x0801F000)	//page 62
//------------------------memory points defines----------------------------------
#define FLASH_POINTS_PAGE	0x0803E800		//0x3E800 / 0x800 = page 125 (0x08020000)//page 64 start (0x0801F800)

#define MEMORY_POINT_SIZE	(12)	//9 bytes include: 1 flag, 4 lat; 4 lon.

#define MEMORY_POINT_FLAG		(0xAA)	//if a mem point exists then it's flag variable is 0xAA
#define MEMORY_POINT_FLAG_POS	(0)
#define MEMORY_POINT_TIME_POS	(1)
#define MEMORY_POINT_LAT_POS	(4)
#define MEMORY_POINT_LON_POS	(8)

#define MEM_POINT_NAME_LEN 	(7)			//max len of any point name
#define MEM_POINT_0_NAME   	("Alarms!")
#define MEM_POINT_1_NAME   	("Group_1")
#define MEM_POINT_2_NAME   	("Group_2")
#define MEM_POINT_3_NAME  	("Group_3")
#define MEM_POINT_4_NAME   	("Group_4")
#define MEM_POINT_5_NAME   	("Group_5")
#define MEM_POINT_6_NAME   	("BcnManu")
#define MEM_POINT_7_NAME  	("Device1")
#define MEM_POINT_8_NAME  	("Device2")
#define MEM_POINT_9_NAME  	("Device3")
#define MEM_POINT_10_NAME  	("Device4")
#define MEM_POINT_11_NAME  	("Device5")

#define MEM_POINTS_GROUPS_ARRAY 	{ 	{MEM_POINT_0_NAME}, 	\
										{MEM_POINT_1_NAME}, 	\
										{MEM_POINT_2_NAME},	 	\
										{MEM_POINT_3_NAME}, 	\
										{MEM_POINT_4_NAME},	 	\
										{MEM_POINT_5_NAME}, 	\
										{MEM_POINT_6_NAME}, 	\
										{MEM_POINT_7_NAME}, 	\
										{MEM_POINT_8_NAME}, 	\
										{MEM_POINT_9_NAME}, 	\
										{MEM_POINT_10_NAME}, 	\
										{MEM_POINT_11_NAME}	}

#define MEM_POINT_NAME_LEN_SHORT	(4)
#define MEM_POINT_0_SHORT   	("Alrm")
#define MEM_POINT_1_SHORT   	("Grp1")
#define MEM_POINT_2_SHORT   	("Grp2")
#define MEM_POINT_3_SHORT  		("Grp3")
#define MEM_POINT_4_SHORT   	("Grp4")
#define MEM_POINT_5_SHORT   	("Grp5")
#define MEM_POINT_6_SHORT   	("BcnM")

#define MEM_POINTS_GROUPS_ARRAY_SHORT 	{ 	{MEM_POINT_0_SHORT}, 	\
										{MEM_POINT_1_SHORT}, 		\
										{MEM_POINT_2_SHORT},	 	\
										{MEM_POINT_3_SHORT}, 		\
										{MEM_POINT_4_SHORT},	 	\
										{MEM_POINT_5_SHORT}, 		\
										{MEM_POINT_6_SHORT}	}
//-----------------------memory points defines end--------------------------
uint8_t settings_array[SETTINGS_SIZE];

struct settings_struct settings;

uint8_t freq_region_values[] = FREQ_REGION_VALUES_ARRAY;
uint8_t coding_rate_values[] = CODING_RATE_VALUES_ARRAY;
uint8_t tx_power_values[] = TX_POWER_VALUES_ARRAY;

void read_flash_page(uint32_t start_address, uint8_t data_array[], uint8_t amount);

uint8_t *get_freq_region_values(void)
{
	return &freq_region_values[0];
}

uint8_t *get_coding_rate_values(void)
{
	return &coding_rate_values[0];
}

uint8_t *get_tx_power_values(void)
{
	return &tx_power_values[0];
}

struct settings_struct *get_settings(void)
{
	return &settings;
}
//struct points_struct **pp_points;
void settings_load(void)
{
	read_flash_page(SETTINGS_PAGE, &settings_array[0], 1);	//0x0803F000

    if (settings_array[SETTINGS_INIT_FLAG_POS] != SETTINGS_INIT_FLAG_DEFAULT)     //if first power-up or FLASH had been erased
    {
        settings_save_default();		//todo: add default load by OK & ESC button hold
    }
        //read from flash
    read_flash_page(SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE);	//0x0803F800
    
    //load settings to struct
    settings.device_number = 					settings_array[SETTINGS_DEVICE_NUMBER_POS];
    settings.devices_on_air = 					settings_array[SETTINGS_DEVICES_ON_AIR_POS];
    settings.spreading_factor = 				settings_array[SETTINGS_SPREADING_FACTOR_POS];
//    settings.freq_region_opt = 					settings_array[SETTINGS_FREQ_REGION_POS];
    settings.coding_rate_opt = 					settings_array[SETTINGS_CODING_RATE_POS];
    settings.freq_channel = 					settings_array[SETTINGS_FREQ_CHANNEL_POS];
    settings.tx_power_opt = 					settings_array[SETTINGS_TX_POWER_POS];
    settings.timeout_threshold = 				settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS];
    settings.fence_threshold = 					settings_array[SETTINGS_FENCE_THRESHOLD_POS];
//    settings.time_zone_dir = 					settings_array[SETTINGS_TIME_ZONE_DIR_POS];
    settings.time_zone_hour = 					settings_array[SETTINGS_TIME_ZONE_HOUR_POS];

    settings.accel_offset_x.as_array[0] =		settings_array[SETTINGS_ACC_OFFSET_X_POS];
    settings.accel_offset_x.as_array[1] =		settings_array[SETTINGS_ACC_OFFSET_X_POS + 1];
    settings.accel_offset_y.as_array[0]	=		settings_array[SETTINGS_ACC_OFFSET_Y_POS];
    settings.accel_offset_y.as_array[1] =		settings_array[SETTINGS_ACC_OFFSET_Y_POS + 1];
    settings.accel_offset_z.as_array[0] =  		settings_array[SETTINGS_ACC_OFFSET_Z_POS];
    settings.accel_offset_z.as_array[1] =		settings_array[SETTINGS_ACC_OFFSET_Z_POS + 1];

    settings.magn_offset_x.as_array[0] =		settings_array[SETTINGS_MAG_OFFSET_X_POS];
    settings.magn_offset_x.as_array[1] =		settings_array[SETTINGS_MAG_OFFSET_X_POS + 1];
    settings.magn_offset_y.as_array[0] =		settings_array[SETTINGS_MAG_OFFSET_Y_POS];
    settings.magn_offset_y.as_array[1] =		settings_array[SETTINGS_MAG_OFFSET_Y_POS + 1];
    settings.magn_offset_z.as_array[0] =		settings_array[SETTINGS_MAG_OFFSET_Z_POS];
    settings.magn_offset_z.as_array[1] =		settings_array[SETTINGS_MAG_OFFSET_Z_POS + 1];

    settings.gyro_offset_x.as_array[0] =		settings_array[SETTINGS_MAG_OFFSET_X_POS];
    settings.gyro_offset_x.as_array[1] =		settings_array[SETTINGS_MAG_OFFSET_X_POS + 1];
    settings.gyro_offset_y.as_array[0] =		settings_array[SETTINGS_MAG_OFFSET_Y_POS];
    settings.gyro_offset_y.as_array[1] =		settings_array[SETTINGS_MAG_OFFSET_Y_POS + 1];
    settings.gyro_offset_z.as_array[0] =		settings_array[SETTINGS_MAG_OFFSET_Z_POS];
    settings.gyro_offset_z.as_array[1] =		settings_array[SETTINGS_MAG_OFFSET_Z_POS + 1];

    settings.accel_radius.as_array[0] = 		settings_array[SETTINGS_ACC_RADIUS_POS];
    settings.accel_radius.as_array[1] = 		settings_array[SETTINGS_ACC_RADIUS_POS + 1];
    settings.magn_radius.as_array[0] = 			settings_array[SETTINGS_MAG_RADIUS_POS];
    settings.magn_radius.as_array[1] = 			settings_array[SETTINGS_MAG_RADIUS_POS + 1];

    if(settings.spreading_factor == 12)
    {
    	settings.device_number = 3;	//RX only (slot1 and slot2), Radio.SetRxConfig IQ inverted, TX on demand
    	settings.devices_on_air = 3;
    	settings.coding_rate_opt = 3;
    }
    else if(settings.spreading_factor == 11) settings.coding_rate_opt = 2;
}

void settings_save_default(void)		//page 63 == 1F800
{
	__disable_irq();
	HAL_FLASH_Unlock();
	FLASH_IF_EraseByPages(123, 1, 0);		//erase_flash_page(SETTINGS_PAGE);0x0803F000

    //assign default values
    settings_array[SETTINGS_INIT_FLAG_POS] = 			SETTINGS_INIT_FLAG_DEFAULT;
    settings_array[SETTINGS_DEVICE_NUMBER_POS] = 		SETTINGS_DEVICE_NUMBER_DEFAULT;
    settings_array[SETTINGS_DEVICES_ON_AIR_POS] = 		SETTINGS_DEVICES_ON_AIR_DEFAULT;
    settings_array[SETTINGS_SPREADING_FACTOR_POS] = 	SETTINGS_SPREADING_FACTOR_DEFAULT;
//    settings_array[SETTINGS_FREQ_REGION_POS] = 			SETTINGS_FREQ_REGION_DEFAULT;
    settings_array[SETTINGS_CODING_RATE_POS] = 			SETTINGS_CODING_RATE_DEFAULT;
    settings_array[SETTINGS_FREQ_CHANNEL_POS] = 		SETTINGS_FREQ_CHANNEL_DEFAULT;
    settings_array[SETTINGS_TX_POWER_POS] = 			SETTINGS_TX_POWER_DEFAULT;
    settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS] = 	SETTINGS_TIMEOUT_THRESHOLD_DEFAULT;
    settings_array[SETTINGS_FENCE_THRESHOLD_POS] = 		SETTINGS_FENCE_THRESHOLD_DEFAULT;
//    settings_array[SETTINGS_TIME_ZONE_DIR_POS] = 		SETTINGS_TIME_ZONE_DIR_DEFAULT;
    settings_array[SETTINGS_TIME_ZONE_HOUR_POS] = 		SETTINGS_TIME_ZONE_HOUR_DEFAULT;


    settings_array[SETTINGS_ACC_OFFSET_X_POS] = 		SETTINGS_ACC_OFFSET_X_LSB_DEFAULT;
    settings_array[SETTINGS_ACC_OFFSET_X_POS + 1] =		SETTINGS_ACC_OFFSET_X_MSB_DEFAULT;
    settings_array[SETTINGS_ACC_OFFSET_Y_POS] = 		SETTINGS_ACC_OFFSET_Y_LSB_DEFAULT;
    settings_array[SETTINGS_ACC_OFFSET_Y_POS + 1] =		SETTINGS_ACC_OFFSET_Y_MSB_DEFAULT;
    settings_array[SETTINGS_ACC_OFFSET_Z_POS] = 		SETTINGS_ACC_OFFSET_Z_LSB_DEFAULT;
    settings_array[SETTINGS_ACC_OFFSET_Z_POS + 1] =		SETTINGS_ACC_OFFSET_Z_MSB_DEFAULT;

    settings_array[SETTINGS_MAG_OFFSET_X_POS] = 		SETTINGS_MAG_OFFSET_X_LSB_DEFAULT;
    settings_array[SETTINGS_MAG_OFFSET_X_POS + 1] =		SETTINGS_MAG_OFFSET_X_MSB_DEFAULT;
    settings_array[SETTINGS_MAG_OFFSET_Y_POS] = 		SETTINGS_MAG_OFFSET_Y_LSB_DEFAULT;
    settings_array[SETTINGS_MAG_OFFSET_Y_POS + 1] =		SETTINGS_MAG_OFFSET_Y_MSB_DEFAULT;
    settings_array[SETTINGS_MAG_OFFSET_Z_POS] = 		SETTINGS_MAG_OFFSET_Z_LSB_DEFAULT;
    settings_array[SETTINGS_MAG_OFFSET_Z_POS + 1] =		SETTINGS_MAG_OFFSET_Z_MSB_DEFAULT;

    settings_array[SETTINGS_GYR_OFFSET_X_POS] = 		SETTINGS_GYR_OFFSET_X_LSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_X_POS + 1] =		SETTINGS_GYR_OFFSET_X_MSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Y_POS] = 		SETTINGS_GYR_OFFSET_Y_LSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Y_POS + 1] =		SETTINGS_GYR_OFFSET_Y_MSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Z_POS] = 		SETTINGS_GYR_OFFSET_Z_LSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Z_POS + 1] =		SETTINGS_GYR_OFFSET_Z_MSB_DEFAULT;

    //write to flash
    FLASH_IF_Write(SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE, NULL);		//write_flash_page(FLASH_SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE);
    HAL_FLASH_Lock();
    __enable_irq();
}

void settings_save(struct settings_struct *p_settings)
{
	__disable_irq();
	HAL_FLASH_Unlock();
	FLASH_IF_EraseByPages(123, 1, 0);		//erase_flash_page(FLASH_SETTINGS_PAGE);0x0803F000

    //assign values
    settings_array[SETTINGS_INIT_FLAG_POS] = 			SETTINGS_INIT_FLAG_DEFAULT;
    settings_array[SETTINGS_DEVICE_NUMBER_POS] = 		p_settings->device_number;
    settings_array[SETTINGS_DEVICES_ON_AIR_POS] =		p_settings->devices_on_air;
    settings_array[SETTINGS_SPREADING_FACTOR_POS] = 	p_settings->spreading_factor;
//    settings_array[SETTINGS_FREQ_REGION_POS] = 			p_settings->freq_region_opt;
    settings_array[SETTINGS_CODING_RATE_POS] = 			p_settings->coding_rate_opt;
    settings_array[SETTINGS_FREQ_CHANNEL_POS] = 		p_settings->freq_channel;
    settings_array[SETTINGS_TX_POWER_POS] = 			p_settings->tx_power_opt;
    settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS] = 	p_settings->timeout_threshold;
    settings_array[SETTINGS_FENCE_THRESHOLD_POS] = 		p_settings->fence_threshold;
//    settings_array[SETTINGS_TIME_ZONE_DIR_POS] = 		p_settings->time_zone_dir;
    settings_array[SETTINGS_TIME_ZONE_HOUR_POS] = 		p_settings->time_zone_hour;

		settings_array[SETTINGS_ACC_OFFSET_X_POS] = 		p_settings->accel_offset_x.as_array[0];
		settings_array[SETTINGS_ACC_OFFSET_X_POS + 1] = 	p_settings->accel_offset_x.as_array[1];
		settings_array[SETTINGS_ACC_OFFSET_Y_POS] = 		p_settings->accel_offset_y.as_array[0];
		settings_array[SETTINGS_ACC_OFFSET_Y_POS + 1] = 	p_settings->accel_offset_y.as_array[1];
 		settings_array[SETTINGS_ACC_OFFSET_Z_POS] = 		p_settings->accel_offset_z.as_array[0];
		settings_array[SETTINGS_ACC_OFFSET_Z_POS + 1] = 	p_settings->accel_offset_z.as_array[1];

		settings_array[SETTINGS_MAG_OFFSET_X_POS] = 		p_settings->magn_offset_x.as_array[0];
		settings_array[SETTINGS_MAG_OFFSET_X_POS + 1] = 	p_settings->magn_offset_x.as_array[1];
		settings_array[SETTINGS_MAG_OFFSET_Y_POS] = 		p_settings->magn_offset_y.as_array[0];
		settings_array[SETTINGS_MAG_OFFSET_Y_POS + 1] = 	p_settings->magn_offset_y.as_array[1];
		settings_array[SETTINGS_MAG_OFFSET_Z_POS] = 		p_settings->magn_offset_z.as_array[0];
		settings_array[SETTINGS_MAG_OFFSET_Z_POS + 1] = 	p_settings->magn_offset_z.as_array[1];

		settings_array[SETTINGS_MAG_OFFSET_X_POS] = 		p_settings->gyro_offset_x.as_array[0];
		settings_array[SETTINGS_MAG_OFFSET_X_POS + 1] = 	p_settings->gyro_offset_x.as_array[1];
		settings_array[SETTINGS_MAG_OFFSET_Y_POS] = 		p_settings->gyro_offset_y.as_array[0];
		settings_array[SETTINGS_MAG_OFFSET_Y_POS + 1] = 	p_settings->gyro_offset_y.as_array[1];
		settings_array[SETTINGS_MAG_OFFSET_Z_POS] = 		p_settings->gyro_offset_z.as_array[0];
		settings_array[SETTINGS_MAG_OFFSET_Z_POS + 1] = 	p_settings->gyro_offset_z.as_array[1];

 		settings_array[SETTINGS_ACC_RADIUS_POS] = 			p_settings->accel_radius.as_array[0];
		settings_array[SETTINGS_ACC_RADIUS_POS + 1] = 		p_settings->accel_radius.as_array[1];
 		settings_array[SETTINGS_MAG_RADIUS_POS] = 			p_settings->magn_radius.as_array[0];
 		settings_array[SETTINGS_MAG_RADIUS_POS + 1] = 		p_settings->magn_radius.as_array[1];
    
    //write to flash
    FLASH_IF_Write(SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE, NULL);		//write_flash_page(FLASH_SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE);
    HAL_Delay(100);
    HAL_FLASH_Lock();
    __enable_irq();

//    memory_points_save();
}

void read_flash_page(uint32_t start_address, uint8_t data_array[], uint8_t amount)
{
	for (uint8_t i = 0; i < amount; i++)
	{
		data_array[i] = ((__IO uint8_t *)start_address)[i];
	}
}

//------------------------memory points----------------------------
uint8_t points_array[1152];	//1024(MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE) (size % sizeof(uint64_t)) != 0)

struct points_struct points[MEMORY_POINTS_TOTAL];        //structures array for devices from 1 to DEVICES_IN_GROUP. Index 0 is invalid and always empty.
struct points_struct *p_points[MEMORY_POINTS_TOTAL];		//structure pointers array
//pp_points = get_points();
struct points_struct **get_points(void)
{
	for (uint8_t point = 0; point < MEMORY_POINTS_TOTAL; point++)
	{
		p_points[point] = &points[point];
	}

	return &p_points[0];
}

struct points_struct **pp_points;

//uint8_t points_array[MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE] = {0}; //uint16 is used because the FLASH organization; actually it is used to carry uint8 data

char points_group_name_values[MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS][MEM_POINT_NAME_LEN + 1] = MEM_POINTS_GROUPS_ARRAY;
char points_group_name_short[MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS][MEM_POINT_NAME_LEN_SHORT + 1] = MEM_POINTS_GROUPS_ARRAY_SHORT;

void read_points_page(uint32_t start_address, uint8_t data_array[], uint16_t amount);

void init_memory_points(void)
{
	pp_points = get_points();
	memory_points_load();
}

void memory_points_load(void)	// FLASH -> buffer array -> devices struct
{
	pp_points = get_points();

	read_points_page(FLASH_POINTS_PAGE, &points_array[0], 1024);	//(MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE)

	for (uint8_t point_group = 0; point_group < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); point_group++)		//MEMORY_POINT_GROUPS
	{
		uint8_t point_group_ind = point_group * MEMORY_SUBPOINTS;

		for (uint8_t point = point_group_ind; point < (point_group_ind + 10); point++)
		{
			uint16_t point_start_index = point * MEMORY_POINT_SIZE;				//0, 16, 32, 48,...
			uint8_t point_number = point;	// + MEMORY_POINT_FIRST;			//0,  1,  2,  3, ...,  9

			if (points_array[point_start_index + MEMORY_POINT_FLAG_POS] == MEMORY_POINT_FLAG)
			{
//				pp_points[point_number]->exist_flag = 1;
				points[point_number].exist_flag = 1;

				for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
				{
//					pp_points[point_number]->latitude.as_array[b] = points_array[point_start_index + MEMORY_POINT_LAT_POS + b];
//					pp_points[point_number]->longitude.as_array[b] = points_array[point_start_index + MEMORY_POINT_LON_POS + b];
					points[point_number].latitude.as_array[b] = points_array[point_start_index + MEMORY_POINT_LAT_POS + b];
					points[point_number].longitude.as_array[b] = points_array[point_start_index + MEMORY_POINT_LON_POS + b];
				}
			}
			else points[point_number].exist_flag = 0;		//pp_points[point_number]->exist_flag = 0;
		}
	}
}
//uint8_t dataTempPage[1024];
void memory_points_save(void)	//struct points_struct **pp_points devices struct -> buffer array -> FLASH (pre-erased)
{
	pp_points = get_points();
	__disable_irq();
	HAL_FLASH_Unlock();
	HAL_Delay(20);
	FLASH_IF_EraseByPages(125, 1, 0);		//erase_flash_page(FLASH_POINTS_PAGE);
	FLASH_IF_EraseByPages(125, 1, 0);
	HAL_Delay(20);

	for (uint8_t point_group = 0; point_group < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); point_group++)		//MEMORY_POINT_GROUPS
	{
		uint8_t point_group_ind = point_group * MEMORY_SUBPOINTS;

		for (uint8_t point = point_group_ind; point < (point_group_ind + 10); point++)
		{
			uint16_t point_start_index = point * MEMORY_POINT_SIZE;			//0, 16, 32, 48, ..., 144
			uint8_t point_number = point;	// + MEMORY_POINT_FIRST;		//0,  1,  2,  3, ...,  9
			if (pp_points[point_number]->exist_flag == 1)
			{
				points_array[point_start_index + MEMORY_POINT_FLAG_POS] = MEMORY_POINT_FLAG;

				for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
				{
					points_array[point_start_index + MEMORY_POINT_LAT_POS + b] = pp_points[point_number]->latitude.as_array[b];
					points_array[point_start_index + MEMORY_POINT_LON_POS + b] = pp_points[point_number]->longitude.as_array[b];
					//todo: add date/time save
				}
			}
			else points_array[point_start_index + MEMORY_POINT_FLAG_POS] = 0;
		}
	}
//	__disable_irq();
//	HAL_FLASH_Unlock();
//	HAL_Delay(10);
	FLASH_IF_Write(FLASH_POINTS_PAGE, &points_array[0], 1152, NULL);	//(MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE)write_flash_page(FLASH_POINTS_PAGE, &raw_points_array[0], MEMORY_POINTS_SIZE);
	HAL_Delay(40);
	HAL_FLASH_Lock();
	HAL_Delay(20);
	__enable_irq();
}

void memory_points_erase(void)
{
	__disable_irq();
	HAL_FLASH_Unlock();
	HAL_Delay(20);
	FLASH_IF_EraseByPages(125, 1, 0);
	FLASH_IF_EraseByPages(125, 1, 0);
	HAL_Delay(20);
	for (uint8_t point_group = 0; point_group < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); point_group++)		//MEMORY_POINT_GROUPS
		{
			uint8_t point_group_ind = point_group * MEMORY_SUBPOINTS;

			for (uint8_t point = point_group_ind; point < (point_group_ind + 10); point++)
			{
				uint16_t point_start_index = point * MEMORY_POINT_SIZE;			//0, 16, 32, 48, ..., 144
				points_array[point_start_index + MEMORY_POINT_FLAG_POS] = 0;
			}
		}
	FLASH_IF_Write(FLASH_POINTS_PAGE, &points_array[0], 1152, NULL);	//(MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE)write_flash_page(FLASH_POINTS_PAGE, &raw_points_array[0], MEMORY_POINTS_SIZE);
	HAL_Delay(40);
	HAL_FLASH_Lock();
	HAL_Delay(20);
	__enable_irq();
}

void save_one_point(int8_t point_absolute_index)
{
	struct settings_struct *p_settings;
	p_settings = get_settings();
	struct devices_struct **pp_devices;
	pp_devices = get_devices();

	points[point_absolute_index].exist_flag = 1;
	points[point_absolute_index].latitude.as_integer = pp_devices[p_settings->device_number]->latitude.as_integer;
	points[point_absolute_index].longitude.as_integer = pp_devices[p_settings->device_number]->longitude.as_integer;
	memory_points_save();		//save to flash
}

void clear_points_group(int8_t current_point_group)
{
	for(int8_t i = 0; i < MEMORY_SUBPOINTS; i++) {
//		point_absolute_index = current_point_group * MEMORY_SUBPOINTS + i;
		points[current_point_group * MEMORY_SUBPOINTS + i].exist_flag = 0;
	}
	memory_points_save();		//save to flash
}

char *get_points_group_name(uint8_t group_number)		//point_number from MEMORY_POINT_FIRST to MEMORY_POINT_LAST
{
	return &points_group_name_values[group_number][0];		// - MEMORY_POINT_FIRST][0];
}

char *get_points_group_short(uint8_t group_number)		//point_number from MEMORY_POINT_FIRST to MEMORY_POINT_LAST
{
	return &points_group_name_short[group_number][0];		// - MEMORY_POINT_FIRST][0];
}

void read_points_page(uint32_t start_address, uint8_t data_array[], uint16_t amount)
{
	for (uint16_t i = 0; i < amount; i++)
	{
		data_array[i] = ((__IO uint8_t *)start_address)[i];
	}
}
//-------------------------memory points end----------------------------
