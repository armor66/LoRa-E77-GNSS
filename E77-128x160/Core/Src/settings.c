#include <string.h>
//#include "config.h"
#include "settings.h"
#include "eeprom.h"
#include "lrns.h"
//#include "flash_if.h"
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

//default values:
#define SETTINGS_INIT_FLAG_DEFAULT      	(0xAA)
#define SETTINGS_DEVICE_NUMBER_DEFAULT  	(3)
#define SETTINGS_DEVICES_ON_AIR_DEFAULT		(3)		//DEVICE_NUMBER_LAST)
#define SETTINGS_SPREADING_FACTOR_DEFAULT	(11)
#define SETTINGS_CODING_RATE_DEFAULT		(CODING_RATE_2_SETTING)		// 2
#define SETTINGS_FREQ_CHANNEL_DEFAULT   	(FREQ_CHANNEL_FIRST)        //base freq is 433.050 and freq step is 25kHz, so CH0 - 433.050 (not valid, not used); CH1 - 433.075 (first LPD channel)
#define SETTINGS_TX_POWER_DEFAULT       	(TX_POWER_10MILLIW_SETTING)	// 2
#define SETTINGS_TIMEOUT_THRESHOLD_DEFAULT  (0)
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

//settings size
#define SETTINGS_SIZE			(0x20) //(32)half-words (% sizeof(uint64_t)) == 0)
#define SETTINGS_PAGE			127
#define SETTINGS_PAGE_ADDR		0x0803F800UL		//page 127

//------------------------lost device defines----------------------------------
#define LOST_DEVICE_PAGE			126
#define LOST_DEVICE_PAGE_ADDR		0x0803F000UL	//page 126
#define LOST_DEVICE_POINTS			7
#define LOST_DEVICE_SIZE			0x40	//64				//divisible by 8(64bit) = (device flag) + 7*(1exist, 4lat, 4lon)

#define LOST_DEVICE_FLAG_POS		(0)

#define LOST_DEVPOINT_EXIST_POS		(0)
#define LOST_DEVPOINT_EXIST_FLAG	(0x56)
#define LOST_DEVPOINT_LAT_POS		(1)
#define LOST_DEVPOINT_LON_POS		(5)
#define LOST_DEVICE_POINT_SIZE		(9)

//------------------------saved groups defines----------------------------------
#define SAVED_GROUPS_PAGE			125
#define SAVED_GROUPS_PAGE_ADDR		0x0803E800UL	//page 125
#define GROUPS_TO_SAVE				7				//0 is emergency & 5 groups 6 is beacon manually saved
#define POINTS_IN_GROUP				7
#define SAVED_GROUP_SIZE			64				//divisible by 8(64bit) = (group flag) + 7*(1exist, 4lat, 4lon)

#define SAVED_GROUP_FLAG_POS		(0)

#define SAVED_POINT_EXIST_POS		(0)
#define SAVED_POINT_EXIST_FLAG		(0x56)
#define SAVED_POINT_LAT_POS			(1)
#define SAVED_POINT_LON_POS			(5)
#define SAVED_POINT_SIZE			(9)

//------------------------memory points defines----------------------------------
#define POINTS_PAGE				124
#define POINTS_PAGE_ADDR		0x0803E000UL		//page 124

#define MEMORY_POINT_SIZE	(16)	//divisible by 8 (64bit) 1 flag, 4 lat; 4 lon.

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

uint8_t coding_rate_values[] = CODING_RATE_VALUES_ARRAY;
uint8_t tx_power_values[] = TX_POWER_VALUES_ARRAY;

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

static uint8_t find_settins(void)
{
//    uint32_t start_addr = SETTINGS_PAGE_ADDR;	// + PAGE_DATA_OFFSET;
//    uint32_t end_addr = SETTINGS_PAGE_ADDR + FLASH_PAGE_SIZE - SETTINGS_SIZE;
    uint8_t init_flag_valid = 0;
    uint8_t end_index = (FLASH_PAGE_SIZE / SETTINGS_SIZE);

	main_flags.settings_address = SETTINGS_PAGE_ADDR + FLASH_PAGE_SIZE ;	//- SETTINGS_SIZE;

//from 64 to 1 (64 iterations)
    for(main_flags.settings_index = end_index; main_flags.settings_index > 0; main_flags.settings_index--)
    {
    	main_flags.settings_address -= SETTINGS_SIZE;						//start from (top of the page - SETTINGS_SIZE)

    	read_page(main_flags.settings_address, &settings_array[SETTINGS_INIT_FLAG_POS], 1);

        if (settings_array[SETTINGS_INIT_FLAG_POS] == SETTINGS_INIT_FLAG_DEFAULT)
        {
        	init_flag_valid = 1;
            break;
        }
//        else main_flags.settings_address -= SETTINGS_SIZE;
    }
    main_flags.settings_index--;
    return init_flag_valid;
}

void settings_load(void)
{
    if(!find_settins())		// (settings_array[SETTINGS_INIT_FLAG_POS] != SETTINGS_INIT_FLAG_DEFAULT)     //if first power-up or FLASH had been erased
    {
    	struct settings_struct *p_settings;
    	p_settings = get_settings();
        settings_save_default(p_settings);		//todo: add default load by OK & ESC button hold
    }
        //read from flash
    read_page(main_flags.settings_address, &settings_array[0], SETTINGS_SIZE);
    
    //load settings to struct
    settings.settings_init_flag = 				settings_array[SETTINGS_INIT_FLAG_POS];
    settings.device_number = 					settings_array[SETTINGS_DEVICE_NUMBER_POS];
    settings.devices_on_air = 					settings_array[SETTINGS_DEVICES_ON_AIR_POS];
    settings.spreading_factor = 				settings_array[SETTINGS_SPREADING_FACTOR_POS];
    settings.coding_rate_opt = 					settings_array[SETTINGS_CODING_RATE_POS];
    settings.freq_channel = 					settings_array[SETTINGS_FREQ_CHANNEL_POS];
    settings.tx_power_opt = 					settings_array[SETTINGS_TX_POWER_POS];
    settings.timeout_threshold = 				settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS];
    settings.fence_threshold = 					settings_array[SETTINGS_FENCE_THRESHOLD_POS];
    settings.time_zone_hour = 					settings_array[SETTINGS_TIME_ZONE_HOUR_POS];

    settings.gyro_offset_x.as_array[0] =		settings_array[SETTINGS_GYR_OFFSET_X_POS];
    settings.gyro_offset_x.as_array[1] =		settings_array[SETTINGS_GYR_OFFSET_X_POS + 1];
    settings.gyro_offset_y.as_array[0] =		settings_array[SETTINGS_GYR_OFFSET_Y_POS];
    settings.gyro_offset_y.as_array[1] =		settings_array[SETTINGS_GYR_OFFSET_Y_POS + 1];
    settings.gyro_offset_z.as_array[0] =		settings_array[SETTINGS_GYR_OFFSET_Z_POS];
    settings.gyro_offset_z.as_array[1] =		settings_array[SETTINGS_GYR_OFFSET_Z_POS + 1];

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

void settings_save_default(struct settings_struct *p_settings)		//page 63 == 1F800
{
    //assign default values
    settings_array[SETTINGS_INIT_FLAG_POS] = 			SETTINGS_INIT_FLAG_DEFAULT;
    settings_array[SETTINGS_DEVICE_NUMBER_POS] = 		SETTINGS_DEVICE_NUMBER_DEFAULT;
    settings_array[SETTINGS_DEVICES_ON_AIR_POS] = 		SETTINGS_DEVICES_ON_AIR_DEFAULT;
    settings_array[SETTINGS_SPREADING_FACTOR_POS] = 	SETTINGS_SPREADING_FACTOR_DEFAULT;
    settings_array[SETTINGS_CODING_RATE_POS] = 			SETTINGS_CODING_RATE_DEFAULT;
    settings_array[SETTINGS_FREQ_CHANNEL_POS] = 		SETTINGS_FREQ_CHANNEL_DEFAULT;
    settings_array[SETTINGS_TX_POWER_POS] = 			SETTINGS_TX_POWER_DEFAULT;
    settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS] = 	SETTINGS_TIMEOUT_THRESHOLD_DEFAULT;
    settings_array[SETTINGS_FENCE_THRESHOLD_POS] = 		SETTINGS_FENCE_THRESHOLD_DEFAULT;
    settings_array[SETTINGS_TIME_ZONE_HOUR_POS] = 		SETTINGS_TIME_ZONE_HOUR_DEFAULT;

    settings_array[SETTINGS_GYR_OFFSET_X_POS] = 		SETTINGS_GYR_OFFSET_X_LSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_X_POS + 1] =		SETTINGS_GYR_OFFSET_X_MSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Y_POS] = 		SETTINGS_GYR_OFFSET_Y_LSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Y_POS + 1] =		SETTINGS_GYR_OFFSET_Y_MSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Z_POS] = 		SETTINGS_GYR_OFFSET_Z_LSB_DEFAULT;
    settings_array[SETTINGS_GYR_OFFSET_Z_POS + 1] =		SETTINGS_GYR_OFFSET_Z_MSB_DEFAULT;

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

    settings_array[SETTINGS_ACC_RADIUS_POS] = 			0;
    settings_array[SETTINGS_ACC_RADIUS_POS + 1] = 		0;
    settings_array[SETTINGS_MAG_RADIUS_POS] = 			0;
    settings_array[SETTINGS_MAG_RADIUS_POS + 1] =		0;
    //to retain calibration data if settings restored to defaults
//	settings_array[SETTINGS_GYR_OFFSET_X_POS] = 		p_settings->gyro_offset_x.as_array[0];
//	settings_array[SETTINGS_GYR_OFFSET_X_POS + 1] = 	p_settings->gyro_offset_x.as_array[1];
//	settings_array[SETTINGS_GYR_OFFSET_Y_POS] = 		p_settings->gyro_offset_y.as_array[0];
//	settings_array[SETTINGS_GYR_OFFSET_Y_POS + 1] = 	p_settings->gyro_offset_y.as_array[1];
//	settings_array[SETTINGS_GYR_OFFSET_Z_POS] = 		p_settings->gyro_offset_z.as_array[0];
//	settings_array[SETTINGS_GYR_OFFSET_Z_POS + 1] = 	p_settings->gyro_offset_z.as_array[1];
//
//	settings_array[SETTINGS_ACC_OFFSET_X_POS] = 		p_settings->accel_offset_x.as_array[0];
//	settings_array[SETTINGS_ACC_OFFSET_X_POS + 1] = 	p_settings->accel_offset_x.as_array[1];
//	settings_array[SETTINGS_ACC_OFFSET_Y_POS] = 		p_settings->accel_offset_y.as_array[0];
//	settings_array[SETTINGS_ACC_OFFSET_Y_POS + 1] = 	p_settings->accel_offset_y.as_array[1];
//	settings_array[SETTINGS_ACC_OFFSET_Z_POS] = 		p_settings->accel_offset_z.as_array[0];
//	settings_array[SETTINGS_ACC_OFFSET_Z_POS + 1] = 	p_settings->accel_offset_z.as_array[1];
//
//	settings_array[SETTINGS_MAG_OFFSET_X_POS] = 		p_settings->magn_offset_x.as_array[0];
//	settings_array[SETTINGS_MAG_OFFSET_X_POS + 1] = 	p_settings->magn_offset_x.as_array[1];
//	settings_array[SETTINGS_MAG_OFFSET_Y_POS] = 		p_settings->magn_offset_y.as_array[0];
//	settings_array[SETTINGS_MAG_OFFSET_Y_POS + 1] = 	p_settings->magn_offset_y.as_array[1];
//	settings_array[SETTINGS_MAG_OFFSET_Z_POS] = 		p_settings->magn_offset_z.as_array[0];
//	settings_array[SETTINGS_MAG_OFFSET_Z_POS + 1] = 	p_settings->magn_offset_z.as_array[1];
//
//	settings_array[SETTINGS_ACC_RADIUS_POS] = 			p_settings->accel_radius.as_array[0];
//	settings_array[SETTINGS_ACC_RADIUS_POS + 1] = 		p_settings->accel_radius.as_array[1];
//	settings_array[SETTINGS_MAG_RADIUS_POS] = 			p_settings->magn_radius.as_array[0];
//	settings_array[SETTINGS_MAG_RADIUS_POS + 1] = 		p_settings->magn_radius.as_array[1];
    //write to flash
	if(flash_unlock())
	{
		flash_erase_page(SETTINGS_PAGE);										//FLASH_IF_EraseByPages(SETTINGS_PAGE, 1, 0);
		flash_write_array(SETTINGS_PAGE_ADDR, settings_array, SETTINGS_SIZE);	//FLASH_IF_Write_Buffer(SETTINGS_PAGE_ADDR, settings_array, SETTINGS_SIZE);
	    flash_lock();
	}
}

void settings_save(struct settings_struct *p_settings)
{
    //assign values
    settings_array[SETTINGS_INIT_FLAG_POS] = 			SETTINGS_INIT_FLAG_DEFAULT;
    settings_array[SETTINGS_DEVICE_NUMBER_POS] = 		p_settings->device_number;
    settings_array[SETTINGS_DEVICES_ON_AIR_POS] =		p_settings->devices_on_air;
    settings_array[SETTINGS_SPREADING_FACTOR_POS] = 	p_settings->spreading_factor;
    settings_array[SETTINGS_CODING_RATE_POS] = 			p_settings->coding_rate_opt;
    settings_array[SETTINGS_FREQ_CHANNEL_POS] = 		p_settings->freq_channel;
    settings_array[SETTINGS_TX_POWER_POS] = 			p_settings->tx_power_opt;
    settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS] = 	p_settings->timeout_threshold;
    settings_array[SETTINGS_FENCE_THRESHOLD_POS] = 		p_settings->fence_threshold;
    settings_array[SETTINGS_TIME_ZONE_HOUR_POS] = 		p_settings->time_zone_hour;

	settings_array[SETTINGS_GYR_OFFSET_X_POS] = 		p_settings->gyro_offset_x.as_array[0];
	settings_array[SETTINGS_GYR_OFFSET_X_POS + 1] = 	p_settings->gyro_offset_x.as_array[1];
	settings_array[SETTINGS_GYR_OFFSET_Y_POS] = 		p_settings->gyro_offset_y.as_array[0];
	settings_array[SETTINGS_GYR_OFFSET_Y_POS + 1] = 	p_settings->gyro_offset_y.as_array[1];
	settings_array[SETTINGS_GYR_OFFSET_Z_POS] = 		p_settings->gyro_offset_z.as_array[0];
	settings_array[SETTINGS_GYR_OFFSET_Z_POS + 1] = 	p_settings->gyro_offset_z.as_array[1];

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

	settings_array[SETTINGS_ACC_RADIUS_POS] = 			p_settings->accel_radius.as_array[0];
	settings_array[SETTINGS_ACC_RADIUS_POS + 1] = 		p_settings->accel_radius.as_array[1];
	settings_array[SETTINGS_MAG_RADIUS_POS] = 			p_settings->magn_radius.as_array[0];
	settings_array[SETTINGS_MAG_RADIUS_POS + 1] = 		p_settings->magn_radius.as_array[1];
    
    //write to flash
	if(flash_unlock())
	{
		main_flags.settings_address += SETTINGS_SIZE;	//write to next free area or erase entire page
		if(main_flags.settings_address ==  SETTINGS_PAGE_ADDR + FLASH_PAGE_SIZE)
		{
			flash_erase_page(SETTINGS_PAGE);	//FLASH_IF_EraseByPages(SETTINGS_PAGE, 1, 0);
			main_flags.settings_address = SETTINGS_PAGE_ADDR;
		}
		flash_write_array((main_flags.settings_address), settings_array, SETTINGS_SIZE);	//FLASH_IF_Write_Buffer(SETTINGS_PAGE_ADDR, settings_array, SETTINGS_SIZE);
	    flash_lock();
	}
}

//------------------------memory points----------------------------
//uint8_t points_array[MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE];	//1024(MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE) (size % sizeof(uint64_t)) != 0)

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

//void read_points_page(uint32_t start_address, uint8_t data_array[], uint16_t amount);
//void init_memory_points(void)
//{
//	pp_points = get_points();
//	memory_points_load();
//}

/*void memory_points_load(void)	// FLASH -> buffer array -> devices struct
{
	pp_points = get_points();

	read_page(POINTS_PAGE_ADDR, &points_array[0], 1024);	//(MEMORY_POINTS_TOTAL * MEMORY_POINT_SIZE)

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
}*/
//uint8_t dataTempPage[1024];
void memory_points_save(void)	//struct points_struct **pp_points devices struct -> buffer array -> FLASH (pre-erased)
{
//	pp_points = get_points();
//	flash_erase_page(POINTS_PAGE);
//
//	for (uint8_t point_group = 0; point_group < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); point_group++)		//MEMORY_POINT_GROUPS
//	{
//		uint8_t point_group_ind = point_group * MEMORY_SUBPOINTS;
//
//		for (uint8_t point = point_group_ind; point < (point_group_ind + 10); point++)
//		{
//			uint16_t point_start_index = point * MEMORY_POINT_SIZE;			//0, 16, 32, 48, ..., 144
//			uint8_t point_number = point;	// + MEMORY_POINT_FIRST;		//0,  1,  2,  3, ...,  9
//			if (pp_points[point_number]->exist_flag == 1)
//			{
//				points_array[point_start_index + MEMORY_POINT_FLAG_POS] = MEMORY_POINT_FLAG;
//
//				for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
//				{
//					points_array[point_start_index + MEMORY_POINT_LAT_POS + b] = pp_points[point_number]->latitude.as_array[b];
//					points_array[point_start_index + MEMORY_POINT_LON_POS + b] = pp_points[point_number]->longitude.as_array[b];
//					//todo: add date/time save
//				}
//			}
//			else points_array[point_start_index + MEMORY_POINT_FLAG_POS] = 0;
//		}
//	}
//	flash_write_array(POINTS_PAGE_ADDR, &points_array[0], 1152);
}

/*void memory_points_erase(void)
{
	flash_erase_page(POINTS_PAGE);

	for (uint8_t point_group = 0; point_group < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); point_group++)		//MEMORY_POINT_GROUPS
		{
			uint8_t point_group_ind = point_group * MEMORY_SUBPOINTS;

			for (uint8_t point = point_group_ind; point < (point_group_ind + 10); point++)
			{
				uint16_t point_start_index = point * MEMORY_POINT_SIZE;			//0, 16, 32, 48, ..., 144
				points_array[point_start_index + MEMORY_POINT_FLAG_POS] = 0;
			}
		}
	flash_write_array(POINTS_PAGE_ADDR, &points_array[0], 1152);
}*/

void save_one_point(int8_t point_absolute_index)
{
	struct settings_struct *p_settings;
	p_settings = get_settings();
	struct devices_struct **pp_devices;
	pp_devices = get_devices();

	points[point_absolute_index].exist_flag = 1;
	points[point_absolute_index].latitude.as_integer = pp_devices[p_settings->device_number]->latitude.as_integer;
	points[point_absolute_index].longitude.as_integer = pp_devices[p_settings->device_number]->longitude.as_integer;
//	memory_points_save();		//save to flash
}

char *get_points_group_name(uint8_t group_number)		//point_number from MEMORY_POINT_FIRST to MEMORY_POINT_LAST
{
	return &points_group_name_values[group_number][0];		// - MEMORY_POINT_FIRST][0];
}

char *get_points_group_short(uint8_t group_number)		//point_number from MEMORY_POINT_FIRST to MEMORY_POINT_LAST
{
	return &points_group_name_short[group_number][0];		// - MEMORY_POINT_FIRST][0];
}
void erase_point_groups(void)
{
	if(flash_unlock())
	{
		flash_erase_page(SAVED_GROUPS_PAGE);
	    flash_lock();
	}
}
void erase_saved_devices(void)
{
	if(flash_unlock())
	{
		flash_erase_page(LOST_DEVICE_PAGE);
    	flash_lock();
	}
}
//-------------------------memory points end-----------------------------
//-------------------------points_groups start----------------------------
const uint8_t saved_group_flag[] = {0xCC, 0xA5, 0xB5, 0xC5, 0xD5, 0xE5, 0xCA};	//emergency & 5 point groups
uint8_t saved_group_array[SAVED_GROUP_SIZE];				//saved_point_group_flag[device] + 63 are 7 points of 9 bytes each
uint8_t saved_group_index;								//FLASH_PAGE_SIZE / SAVED_GROUP_SIZE (from 1 to 32)
uint32_t saved_group_address[GROUPS_TO_SAVE];				//page start address + SAVED_GROUP_SIZE incremented

static uint8_t find_saved_group(uint8_t group)
{
	struct point_groups_struct **pp_point_groups;
	pp_point_groups = get_point_groups();

	uint8_t group_found = 0;
	uint8_t end_index = (FLASH_PAGE_SIZE / SAVED_GROUP_SIZE);
//search from top of the page
	saved_group_address[group] = SAVED_GROUPS_PAGE_ADDR + FLASH_PAGE_SIZE;	// - SAVED_GROUP_SIZE;	//max available

//from 32 to 1 (32 iterations)
    for(saved_group_index = end_index; saved_group_index > 0; saved_group_index--)
    {
    	saved_group_address[group] -= SAVED_GROUP_SIZE;		//start from (top of the page - SAVED_GROUP_SIZE)

    	read_page(saved_group_address[group], &saved_group_array[SAVED_GROUP_FLAG_POS], 1);

    	if (saved_group_array[SAVED_GROUP_FLAG_POS] == saved_group_flag[group])
        {
    		group_found = 1;
            break;
        }
//        else saved_group_address[group] -= SAVED_GROUP_SIZE;
    }
    pp_point_groups[group]->index_in_flash = saved_group_index - 1;
    return group_found;
}

void saved_group_load(uint8_t group)
{
//	struct point_groups_struct **pp_point_groups;
//	pp_point_groups = get_point_groups();
	pp_points = get_points();
    if(find_saved_group(group))
    {
//read from flash, fill lost_device_array with 64 bytes. saved_group_flag[group] + 63 are 7 points of 9 bytes each
    	read_page(saved_group_address[group], &saved_group_array[0], SAVED_GROUP_SIZE);

    	for(uint8_t point = 0; point < POINTS_IN_GROUP; point++)		//read lost_device_array[64]
    	{	//exist flag index
    	   	uint8_t read_from_flash_point_index = point*SAVED_POINT_SIZE + 1; 	//1, 10, 19, 28, 37, 46, 55
    	   	uint8_t global_point_index = group * MEMORY_SUBPOINTS + point + 1;
/*group*****global_point_zero index*****global_point_index*****point*****read_from_flash_point_index*/
//	0	(0)					0				 1-7				1-7			1, 10, 19, 28, 37, 46, 55
//	1	(1)				 	8				 9-15				1-7			1, 10, 19, 28, 37, 46, 55
//	2 	(2)					16				17-23				1-7			1, 10, 19, 28, 37, 46, 55
//	3 	(3)					24				25-31				1-7			1, 10, 19, 28, 37, 46, 55
//	4 	(4)					32				33-39				1-7			1, 10, 19, 28, 37, 46, 55
//	5 	(5)					40				41-47				1-7			1, 10, 19, 28, 37, 46, 55
//bcnman(6)					48				49-55
    		if (saved_group_array[read_from_flash_point_index] == SAVED_POINT_EXIST_FLAG)
    		{
    			points[global_point_index].exist_flag = 1;

    			for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
    			{
					points[global_point_index].latitude.as_array[b] = saved_group_array[read_from_flash_point_index + LOST_DEVPOINT_LAT_POS + b];
					points[global_point_index].longitude.as_array[b] = saved_group_array[read_from_flash_point_index + LOST_DEVPOINT_LON_POS + b];
    			}
    		}else points[global_point_index].exist_flag = 0;	//pp_lost_device[device][point].exist_flag = 0;
    	}
    }else
    {
//    	pp_point_groups[group]->index_in_flash = -1;
    	return;
    }
}

void points_group_save(uint8_t group)
{
	struct point_groups_struct **pp_point_groups;
	pp_point_groups = get_point_groups();

	pp_points = get_points();
	uint8_t saved_group_found = 0;
	uint8_t max_index = 0;
	uint32_t max_address = 0;

	memset(saved_group_array, 0, SAVED_GROUP_SIZE);

//find max index/address to save new group_array in next free LOST_DEVICE_SIZE area
    for (uint8_t grp = 0; grp < GROUPS_TO_SAVE; grp++)
    {
    	if(find_saved_group(grp))
    	{
    		saved_group_found +=1;
        	if(saved_group_index > max_index)
        	{
        		max_index = saved_group_index;
        		max_address = saved_group_address[grp];
        	}
    	}else pp_point_groups[grp]->index_in_flash = -1;
    }

	//fill group_array[64]
   	saved_group_array[SAVED_GROUP_FLAG_POS] = saved_group_flag[group];	    //fill this after find procedure

	for(uint8_t point = 0; point < POINTS_IN_GROUP; point++)
	{
	   	uint8_t write_to_flash_point_index = point*SAVED_POINT_SIZE + 1; 	//1, 10, 19, 28, 36, 46, 55
	   	uint8_t global_point_index = group * MEMORY_SUBPOINTS + point + 1;
/*group*****global_point_zero index*****global_point_index*****point*****write_to_flash_point_index*/
//	0	(0)					0				 1-7				1-7			1, 10, 19, 28, 37, 46, 55
//	1	(1)				 	8				 9-15				1-7			1, 10, 19, 28, 37, 46, 55
//	2 	(2)					16				17-23				1-7			1, 10, 19, 28, 37, 46, 55
//	3 	(3)					24				25-31				1-7			1, 10, 19, 28, 37, 46, 55
//	4 	(4)					32				33-39				1-7			1, 10, 19, 28, 37, 46, 55
//	5 	(5)					40				41-47				1-7			1, 10, 19, 28, 37, 46, 55
//bcnman(6)					48				49-55				1-7			1, 10, 19, 28, 37, 46, 55
		if(pp_points[global_point_index]->exist_flag == 1)
		{
			saved_group_array[write_to_flash_point_index] = SAVED_POINT_EXIST_FLAG;

			for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
   			{
				saved_group_array[write_to_flash_point_index + LOST_DEVPOINT_LAT_POS + b] = pp_points[global_point_index]->latitude.as_array[b];
				saved_group_array[write_to_flash_point_index + LOST_DEVPOINT_LON_POS + b] = pp_points[global_point_index]->longitude.as_array[b];
   			}
		}
	}

   	if(flash_unlock())	//write to next free area or erase entire page
   	{
   		if((max_index > (FLASH_PAGE_SIZE / SAVED_GROUP_SIZE) - 1) || !saved_group_found)	//the page is full if last area occupied (index = 32)
   		{
   			flash_erase_page(SAVED_GROUPS_PAGE);
//todo: save and flash all saved groups from the beginning of SAVED_GROUPS_PAGE_ADDR
   			saved_group_address[group] = SAVED_GROUPS_PAGE_ADDR;			//start from first page address
   		}
   		else saved_group_address[group] = max_address + SAVED_GROUP_SIZE;	//flash to next free area

   	    pp_point_groups[group]->index_in_flash = max_index;

   		flash_write_array(saved_group_address[group], saved_group_array, SAVED_GROUP_SIZE);	//FLASH_IF_Write_Buffer(SETTINGS_PAGE_ADDR, settings_array, SETTINGS_SIZE);
   	    flash_lock();
   	}
}

void clear_points_group(int8_t group)
{
	pp_points = get_points();
	uint8_t saved_group_found = 0;
	uint8_t max_index = 0;
	uint32_t max_address = 0;

	//find max index/address to save new group_array in next free LOST_DEVICE_SIZE area
    for (uint8_t grp = 0; grp < GROUPS_TO_SAVE; grp++)
    {
    	if(find_saved_group(grp))
    	{
    		if(group == grp) saved_group_found = 1;
        	if(saved_group_index > max_index)
        	{
        		max_index = saved_group_index;
        		max_address = saved_group_address[grp];
        	}
    	}//else pp_devices[dev]->index_in_flash = -1;
    }
/*if saved group has found we should create new 64 bytes area with it "saved_group_flag[group]"
* valid at "0" position and fill all point indexes (1, 10, 19, 28, 37, 46, 55) with "0"*/
    if(saved_group_found)	//if chosen group not found, do nothing
    {
    	//fill group_array[64]
    	for(uint8_t point = 0; point < POINTS_IN_GROUP; point++)
    	{
    	   	uint8_t write_to_flash_point_index = point*SAVED_POINT_SIZE + 1;	//0 to 63 array
    		saved_group_array[write_to_flash_point_index] = 0;				//1, 10, 19, 28, 37, 46, 55
    	}
//    	memset(saved_group_array, 0, SAVED_GROUP_SIZE);

    	//fill this after find procedure
    	saved_group_array[SAVED_GROUP_FLAG_POS] = saved_group_flag[group];
    	if(flash_unlock())	//write to next free area or erase entire page
    	{
       		if(max_index > (FLASH_PAGE_SIZE / SAVED_GROUP_SIZE) - 1)	//the page is full if last area occupied (index = 32)
       		{
       			flash_erase_page(SAVED_GROUPS_PAGE);
//todo: save and flash all saved groups from the beginning of SAVED_GROUPS_PAGE_ADDR
       		}
       		else
       		{	//write to the next free area point exist indexes with zeroes
       			saved_group_address[group] = max_address + SAVED_GROUP_SIZE;
       	   		flash_write_array(saved_group_address[group], saved_group_array, SAVED_GROUP_SIZE);
       		}
       	    flash_lock();
    	}
    }
}
//------------------------lost_devices start----------------------------
const uint8_t lost_device_flag[] = {0x00, 0xA5, 0xB5, 0xC5, 0xD5, 0xE5};		//5 devices max
uint8_t lost_device_array[LOST_DEVICE_SIZE];				//lost_device_flag[device] + 63 are 7 points of 9 bytes each
uint8_t lost_device_index;										//FLASH_PAGE_SIZE / LOST_DEVICE_SIZE (from 1 to 32)
uint32_t lost_device_address[DEVICES_ON_AIR_MAX + 1];			//page start address + LOST_DEVICE_SIZE incremented

static uint8_t find_lost_device(uint8_t device)
{
	struct devices_struct **pp_devices;
	pp_devices = get_devices();
	uint8_t device_found = 0;
	uint8_t end_index = (FLASH_PAGE_SIZE / LOST_DEVICE_SIZE);

	lost_device_address[device] = LOST_DEVICE_PAGE_ADDR + FLASH_PAGE_SIZE;	// - LOST_DEVICE_SIZE;	//top of the page

//from 32 to 1 (32 iterations)
    for(lost_device_index = end_index; lost_device_index > 0; lost_device_index--)
    {
    	lost_device_address[device] -= LOST_DEVICE_SIZE;	//start from (top of the page - LOST_DEVICE_SIZE)

    	read_page(lost_device_address[device], &lost_device_array[LOST_DEVICE_FLAG_POS], 1);

    	if(lost_device_array[LOST_DEVICE_FLAG_POS] == lost_device_flag[device])	//for device specific flag
        {
    		device_found = 1;
//    	    pp_devices[device]->index_in_flash = lost_device_index - 1;
            break;
        }
//      else lost_device_address[device] -= LOST_DEVICE_SIZE;
    }
    pp_devices[device]->index_in_flash = lost_device_index - 1;
    return device_found;
}

void lost_device_load(uint8_t device)
{
//	struct devices_struct **pp_devices;
//	pp_devices = get_devices();
	pp_points = get_points();
    if(find_lost_device(device))
    {
//read from flash, fill lost_device_array with 64 bytes. lost_device_flag[device] + 63 are 7 points of 9 bytes each
    	read_page(lost_device_address[device], &lost_device_array[0], LOST_DEVICE_SIZE);

    	for(uint8_t point = 0; point < LOST_DEVICE_POINTS; point++)		//read lost_device_array[64]
    	{	//exist flag index
    	   	uint8_t read_from_flash_point_index = point*LOST_DEVICE_POINT_SIZE + 1; 	//1, 10, 19, 28, 37, 46, 55
    	   	uint8_t global_point_index = (GROUPS_TO_SAVE + device - 1) * MEMORY_SUBPOINTS + point + 1;
/*device*****global_point_zero index*****global_point_index*****point*****read_from_flash_point_index*/
//	1	(7)				 	56				57-63				1-7			1, 10, 19, 28, 37, 46, 55
//	2 	(8)					64				65-71				1-7			1, 10, 19, 28, 37, 46, 55
//	3 	(9)					72				73-79				1-7			1, 10, 19, 28, 37, 46, 55
//	4  (10)					80				81-87				1-7			1, 10, 19, 28, 37, 46, 55
//	5  (11)					88				89-95				1-7			1, 10, 19, 28, 37, 46, 55
    		if (lost_device_array[read_from_flash_point_index] == LOST_DEVPOINT_EXIST_FLAG)
    		{
    			points[global_point_index].exist_flag = 1;

    			for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
    			{
					points[global_point_index].latitude.as_array[b] = lost_device_array[read_from_flash_point_index + LOST_DEVPOINT_LAT_POS + b];
					points[global_point_index].longitude.as_array[b] = lost_device_array[read_from_flash_point_index + LOST_DEVPOINT_LON_POS + b];
    			}
    		}else points[global_point_index].exist_flag = 0;	//pp_lost_device[device][point].exist_flag = 0;
    	}
    }else
    {
//    	pp_devices[device]->index_in_flash = -1;
    	return;
    }
}

void lost_device_save(uint8_t device)	// executed when "case 3" TIM1-IRQ occurs
{
	struct devices_struct **pp_devices;
	pp_devices = get_devices();
	pp_points = get_points();
	uint8_t lost_device_found = 0;
	uint8_t max_index = 0;
	uint32_t max_address = 0;

	memset(lost_device_array, 0, LOST_DEVICE_SIZE);

	//find max index/address to save new lost_device_array in next free LOST_DEVICE_SIZE area
    for (uint8_t dev = 1; dev <= DEVICES_ON_AIR_MAX; dev++)
    {
    	if(find_lost_device(dev))
    	{
    		lost_device_found +=1;
        	if(lost_device_index > max_index)
        	{
        		max_index = lost_device_index;
        		max_address = lost_device_address[dev];
        	}
    	}else pp_devices[dev]->index_in_flash = -1;
    }

    //fill lost_device_array[64]
	lost_device_array[LOST_DEVICE_FLAG_POS] = lost_device_flag[device];		//fill this after find procedure

	for(uint8_t point = 0; point < LOST_DEVICE_POINTS; point++)
	{
	   	uint8_t write_to_flash_point_index = point*LOST_DEVICE_POINT_SIZE + 1; 	//1, 10, 19, 28, 37, 46, 55
	   	uint8_t global_point_index = (GROUPS_TO_SAVE + device - 1) * MEMORY_SUBPOINTS + point + 1;
/*device*****global_point_zero index*****global_point_index*****point*****write_to_flash_point_index*/
//	1	(7)				 	56				57-63				1-7			1, 10, 19, 28, 37, 46, 55
//	2 	(8)					64				65-71				1-7			1, 10, 19, 28, 37, 46, 55
//	3 	(9)					72				73-79				1-7			1, 10, 19, 28, 37, 46, 55
//	4  (10)					80				81-87				1-7			1, 10, 19, 28, 37, 46, 55
//	5  (11)					88				89-95				1-7			1, 10, 19, 28, 37, 46, 55
		if(pp_points[global_point_index]->exist_flag == 1)
		{
			lost_device_array[write_to_flash_point_index] = LOST_DEVPOINT_EXIST_FLAG;

			for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
   			{
				lost_device_array[write_to_flash_point_index + LOST_DEVPOINT_LAT_POS + b] = pp_points[global_point_index]->latitude.as_array[b];
				lost_device_array[write_to_flash_point_index + LOST_DEVPOINT_LON_POS + b] = pp_points[global_point_index]->longitude.as_array[b];
   			}
		}
	}

   	if(flash_unlock())	//write to next free area or erase entire page
   	{
   		if((max_index > (FLASH_PAGE_SIZE / LOST_DEVICE_SIZE) - 1) || !lost_device_found)	//the page is full if last area occupied (index = 32)
   		{
   			flash_erase_page(LOST_DEVICE_PAGE);
//todo: save and flash all existed lost devices from the beginning of LOST_DEVICE_PAGE_ADDR
   			lost_device_address[device] = LOST_DEVICE_PAGE_ADDR;
//   			max_index = 99;
   		}
   		else lost_device_address[device] = max_address + LOST_DEVICE_SIZE;

   		pp_devices[device]->index_in_flash = max_index;

   		flash_write_array(lost_device_address[device], lost_device_array, LOST_DEVICE_SIZE);	//FLASH_IF_Write_Buffer(SETTINGS_PAGE_ADDR, settings_array, SETTINGS_SIZE);
   	    flash_lock();
   	}
}

