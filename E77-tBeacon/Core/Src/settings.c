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
#define SETTINGS_DEVICE_NUMBER_DEFAULT  	(2)
#define SETTINGS_DEVICES_ON_AIR_DEFAULT		(3)		//DEVICE_NUMBER_LAST)
#define SETTINGS_SPREADING_FACTOR_DEFAULT	(11)
#define SETTINGS_CODING_RATE_DEFAULT		(CODING_RATE_2_SETTING)		// 2
#define SETTINGS_FREQ_CHANNEL_DEFAULT   	(FREQ_CHANNEL_FIRST)        //base freq is 433.050 and freq step is 25kHz, so CH0 - 433.050 (not valid, not used); CH1 - 433.075 (first LPD channel)
#define SETTINGS_TX_POWER_DEFAULT       	(TX_POWER_10MILLIW_SETTING)	// 2
#define SETTINGS_TIMEOUT_THRESHOLD_DEFAULT  (0)
#define SETTINGS_FENCE_THRESHOLD_DEFAULT  	(0)
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

uint8_t find_settins(void)
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
//    	settings.device_number = 3;	//RX only (slot1 and slot2), Radio.SetRxConfig IQ inverted, TX on demand
    	settings.devices_on_air = 3;
    	settings.coding_rate_opt = 3;
    }
    else if(settings.spreading_factor == 11) settings.coding_rate_opt = 2;
}

void settings_save_default(struct settings_struct *settings)
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
		if(main_flags.settings_address == SETTINGS_PAGE_ADDR + FLASH_PAGE_SIZE)
		{
			flash_erase_page(SETTINGS_PAGE);	//FLASH_IF_EraseByPages(SETTINGS_PAGE, 1, 0);
			main_flags.settings_address = SETTINGS_PAGE_ADDR;
		}
		flash_write_array((main_flags.settings_address), settings_array, SETTINGS_SIZE);	//FLASH_IF_Write_Buffer(SETTINGS_PAGE_ADDR, settings_array, SETTINGS_SIZE);
	    flash_lock();
	}
}
