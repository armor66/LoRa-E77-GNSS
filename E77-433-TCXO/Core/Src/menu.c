#include <math.h>
#include <string.h>
#include "menu.h"
#include "ST7735.h"
#include <stdio.h>			//for 'sprintf'
#include "subghz_phy_app.h"
#include "sys_app.h"
#include "buttons.h"
//#include "lsm303.h"
#include "gpio.h"		//#include "main.h"
#include "timer.h"
#include "settings.h"
#include "lrns.h"
#include "compass.h"
#include "adc.h"
#include "tim.h"
#include "lptim.h"
#include "gnss.h"

char Line[24][32];	//[14][24]

char *fixType[] = {"NoFix", "DReck", "2DFix", "3DFix", "DReck", "Time"}; 	//only 2 bits used to transmit (first 4 items)

uint8_t points_group_ind = 0;

int16_t north = 0;
int16_t north_old = 0;
int16_t azimuth_relative_deg = 0;
double north_rad = 0;
double north_rad_old = 0;
int16_t scaled_dist;
int16_t distance_old[10];
double azimuth_relative_rad;
double azimuth_relative_rad_old[10];

//-----------------------------MEMORY POINTS MENU----------------------------
uint8_t memory_subpoint = 0;
uint8_t memory_subpoint_ind[MEMORY_POINT_GROUPS];
uint8_t flag_points_changed = 0;

uint8_t get_current_item(void);
uint8_t get_last_item(void);
void set_current_item(uint8_t new_value);
void reset_current_item_in_menu(uint8_t menu);

void scroll_up(void);
void scroll_down(void);
void switch_forward(void);
void switch_backward(void);

// MAIN MENU
void draw_main(void);
void main_ok(void);
// NAVIGATION MENU

//SUB NAVIGATION MENU
void draw_navigation(void);
void draw_beacons(void);
void navigation_up(void);
void navigation_down(void);
void navigation_ok(void);
//void navigation_esc(void);
void beacons_ok(void);
void beacons_esc(void);

void draw_navto_points(void);
void navto_points_ok(void);
void navto_points_esc(void);
// DEVICES MENU
void draw_devices(void);
void draw_this_device(void);
void draw_device_submenu(void);
void draw_set_points(void);
//SUB POINTS MENU
void set_subpoint_up(void);
void set_subpoint_down(void);
void set_subpoint_ok(void);
void set_subpoint_esc(void);
//SUB DEVICES MENU
void scroll_devices_up(void);
void scroll_devices_down(void);
void devices_ok(void);
void devices_esc(void);
// POINTS MENU
void draw_points(void);
void points_esc(void);
void draw_show_points(void);
// SETTINGS MENU
void draw_settings(void);
void draw_set_settings(void);
//SUB SETTINGS MENU :0
void set_device_number_up(void);
void set_device_number_down(void);
void set_device_number_ok(void);
void set_device_number_esc(void);
//:1
void set_spreading_factor_up(void);
void set_spreading_factor_down(void);
void set_spreading_factor_ok(void);
void set_spreading_factor_esc(void);
//:2
//void set_freq_region_up(void);
//void set_freq_region_down(void);
//void set_freq_region_ok(void);
//void set_freq_region_esc(void);
//:2
void set_coding_rate_up(void);
void set_coding_rate_down(void);
void set_coding_rate_ok(void);
void set_coding_rate_esc(void);
//:3
void set_freq_channel_up(void);
void set_freq_channel_down(void);
void set_freq_channel_ok(void);
void set_freq_channel_esc(void);
//:4
void set_tx_power_up(void);
void set_tx_power_down(void);
void set_tx_power_ok(void);
void set_tx_power_esc(void);
//:5
void set_timezone_up(void);
void set_timezone_down(void);
void set_timezone_ok(void);
void set_timezone_esc(void);
//:6
void set_timeout_up(void);
void set_timeout_down(void);
void set_timeout_ok(void);
void set_timeout_esc(void);
//:7
void set_fence_up(void);
void set_fence_down(void);
void set_fence_ok(void);
void set_fence_esc(void);
// SAVE SETTINGS
//void settings_ok(void);
void draw_confirm_settings(void);
void confirm_settings_reboot(void);
void confirm_settings_restore(void);
//ACTIONS MENU
void draw_actions(void);
void power_long(void);
//void actions_up(void);
//void actions_down(void);
void actions_ok(void);
void actions_esc(void);
void draw_restore_defaults(void);
void set_brightness(void);
//ALL MENUS HERE
enum
{
    //menu number starts from 1, because 0 is used as "end marker" in menu structs
    M_MAIN = 1,
    M_NAVIGATION,
	M_BEACONS,

	M_DEVICES,
	M_DEVICE_SUBMENU,
	M_SET_POINTS,

	M_NAVTO_POINTS,

	M_POINTS,
	M_POINTS_SUBMENU,
	M_POINTS_SUBMENU_DEVICES,

	M_SETTINGS,
	M_CONFIRM_SETTINGS,

	M_SET_DEVICE_NUMBER,
	M_SET_SPREADING_FACTOR,

//	M_SET_FREQ_REGION,
	M_SET_CODING_RATE,
    M_SET_FREQ_CHANNEL,
    M_SET_TX_POWER,

	M_SET_TIMEZONE,
	M_SET_TIMEOUT,
	M_SET_FENCE,

	M_ACTIONS,
	M_CONFIRM_RESTORING
};

//ALL MENU ITEMS HERE (for each menu separately)
//note: for all menus first item always has index of 0
#define M_ALL_I_FIRST (0)

//MAIN
enum
{
    M_MAIN_I_NAVIGATION = 0,
	M_MAIN_I_DEVICES,
	M_MAIN_I_NAVTO_POINTS,
	M_MAIN_I_POINTS,
	M_MAIN_I_SETTINGS,
	M_MAIN_I_ACTIONS,					//last item
    M_MAIN_I_LAST = M_MAIN_I_ACTIONS	//copy last item here
};

//DEVICES
//enum
//{
//    M_DEVICES_I_DEVICES = 0,
//	M_DEVICES_I_POINTS,					//last item
//    M_DEVICES_I_LAST = M_DEVICES_I_POINTS	//copy last item here
//};

//POWER SUBMENU
enum
{
	M_ACTIONS_I_POWER_OFF = 0,
    M_ACTIONS_I_EMERGENCY,
	M_ACTIONS_I_ALARM,
	M_ACTIONS_I_GATHER,
	M_ACTIONS_I_ERASE_POINTS,
	M_ACTIONS_I_SET_DEFAULTS,
	M_ACTIONS_I_BEEPER,
	M_ACTIONS_I_FLWTREK,						//last item
	M_ACTIONS_I_LAST = M_ACTIONS_I_FLWTREK		//copy last item here
};

//POINTS
enum
{
	M_POINTS_I_ALARMS = 0,
	M_POINTS_I_GROUP1,
	M_POINTS_I_GROUP2,
	M_POINTS_I_GROUP3,
	M_POINTS_I_GROUP4,
	M_POINTS_I_GROUP5,
	M_POINTS_I_BCNMAN,
	M_POINTS_I_SAVED1,
	M_POINTS_I_SAVED2,
	M_POINTS_I_SAVED3,
	M_POINTS_I_SAVED4,
	M_POINTS_I_SAVED5,
	M_POINTS_I_LAST = M_POINTS_I_SAVED5
};

enum
{
	M_DEVICE_SUBMENU_I_ALARMS = 0,
	M_DEVICE_SUBMENU_I_GROUP1,
	M_DEVICE_SUBMENU_I_GROUP2,
	M_DEVICE_SUBMENU_I_GROUP3,
	M_DEVICE_SUBMENU_I_GROUP4,
	M_DEVICE_SUBMENU_I_GROUP5,
	M_DEVICE_SUBMENU_I_BCNMAN,
//	M_DEVICE_SUBMENU_I_BCNAUT,
	M_DEVICE_SUBMENU_I_LAST = M_DEVICE_SUBMENU_I_BCNMAN			//copy last item here
};
//SETTINGS
enum
{
    M_SETTINGS_I_DEVICE = 0,		// + AMOUNT
	M_SETTINGS_I_SPREADING_FACTOR,	// + MODULATION lora/fsk
//	M_SETTINGS_I_FREQ_REGION,
	M_SETTINGS_I_CODING_RATE,
	M_SETTINGS_I_FREQ_CHANNEL,
	M_SETTINGS_I_TX_POWER,
	M_SETTINGS_I_TIMEZONE,
	M_SETTINGS_I_TIMEOUT,
	M_SETTINGS_I_FENCE,
    M_SETTINGS_I_LAST = M_SETTINGS_I_FENCE		//copy last item here
};

//Only exclusive (non default) actions here, for example edit a variable in settings
const struct
{
    uint8_t current_menu;
    uint8_t button_pressed;
    void (*execute_function)(void);
} menu_exclusive_table[] = 
{
//  Current Menu                Button pressed          Action (function name)

	{M_MAIN,					BTN_OK,					main_ok},
	{M_MAIN,					BTN_ESC,				set_brightness},

	{M_NAVIGATION,				BTN_UP,					navigation_up},		//change distance range
	{M_NAVIGATION,				BTN_DOWN,				navigation_down},	//change distance range
	{M_NAVIGATION,				BTN_OK,					navigation_ok},		//other points on loop
//	{M_NAVIGATION,				BTN_ESC,				navigation_esc},	//to main menu

	{M_BEACONS,					BTN_UP,					navigation_up},
	{M_BEACONS,					BTN_DOWN,				navigation_down},
	{M_BEACONS,					BTN_OK,					beacons_ok},
	{M_BEACONS,					BTN_ESC,				beacons_esc},

	{M_DEVICES,					BTN_UP,					scroll_devices_up},
	{M_DEVICES,					BTN_DOWN,				scroll_devices_down},
	{M_DEVICES,					BTN_OK,					devices_ok},
	{M_DEVICES,					BTN_ESC,				devices_esc},

	{M_SET_POINTS,				BTN_UP,                 set_subpoint_up},
	{M_SET_POINTS,				BTN_DOWN,               set_subpoint_down},
	{M_SET_POINTS,				BTN_OK,                 set_subpoint_ok},
	{M_SET_POINTS,				BTN_ESC,            	set_subpoint_esc},

	{M_NAVTO_POINTS,			BTN_UP,					navigation_up},
	{M_NAVTO_POINTS,			BTN_DOWN,				navigation_down},
	{M_NAVTO_POINTS,			BTN_OK,					navto_points_ok},
	{M_NAVTO_POINTS,			BTN_ESC,				navto_points_esc},

	{M_SET_DEVICE_NUMBER,		BTN_UP,                 set_device_number_up},
	{M_SET_DEVICE_NUMBER,		BTN_DOWN,               set_device_number_down},
	{M_SET_DEVICE_NUMBER,		BTN_OK,                 set_device_number_ok},
	{M_SET_DEVICE_NUMBER,		BTN_ESC,            	set_device_number_esc},

	{M_SET_SPREADING_FACTOR,	BTN_UP,                 set_spreading_factor_up},
	{M_SET_SPREADING_FACTOR,	BTN_DOWN,               set_spreading_factor_down},
	{M_SET_SPREADING_FACTOR,	BTN_OK,                 set_spreading_factor_ok},
	{M_SET_SPREADING_FACTOR,	BTN_ESC,            	set_spreading_factor_esc},

//	{M_SET_FREQ_REGION,			BTN_UP,                 set_freq_region_up},
//	{M_SET_FREQ_REGION,			BTN_DOWN,               set_freq_region_down},
//	{M_SET_FREQ_REGION,			BTN_OK,                 set_freq_region_ok},
//	{M_SET_FREQ_REGION,			BTN_ESC,            	set_freq_region_esc},
	{M_SET_CODING_RATE,			BTN_UP,                 set_coding_rate_up},
	{M_SET_CODING_RATE,			BTN_DOWN,               set_coding_rate_down},
	{M_SET_CODING_RATE,			BTN_OK,                 set_coding_rate_ok},
	{M_SET_CODING_RATE,			BTN_ESC,            	set_coding_rate_esc},

	{M_SET_FREQ_CHANNEL,		BTN_UP,                 set_freq_channel_up},
	{M_SET_FREQ_CHANNEL,		BTN_DOWN,               set_freq_channel_down},
	{M_SET_FREQ_CHANNEL,		BTN_OK,                 set_freq_channel_ok},
	{M_SET_FREQ_CHANNEL,		BTN_ESC,            	set_freq_channel_esc},

	{M_SET_TX_POWER,			BTN_UP,                 set_tx_power_up},
	{M_SET_TX_POWER,			BTN_DOWN,               set_tx_power_down},
	{M_SET_TX_POWER,			BTN_OK,                 set_tx_power_ok},
	{M_SET_TX_POWER,			BTN_ESC,                set_tx_power_esc},

	{M_SET_TIMEZONE,			BTN_UP,					set_timezone_up},
	{M_SET_TIMEZONE,			BTN_DOWN,				set_timezone_down},
	{M_SET_TIMEZONE,			BTN_OK,					set_timezone_ok},
	{M_SET_TIMEZONE,			BTN_ESC,				set_timezone_esc},

	{M_SET_TIMEOUT,				BTN_UP,                 set_timeout_up},
	{M_SET_TIMEOUT,				BTN_DOWN,               set_timeout_down},
	{M_SET_TIMEOUT,				BTN_OK,                 set_timeout_ok},
	{M_SET_TIMEOUT,				BTN_ESC,            	set_timeout_esc},

	{M_SET_FENCE,				BTN_UP,                 set_fence_up},
	{M_SET_FENCE,				BTN_DOWN,               set_fence_down},
	{M_SET_FENCE,				BTN_OK,                 set_fence_ok},
	{M_SET_FENCE,				BTN_ESC,            	set_fence_esc},
	{M_CONFIRM_SETTINGS,		BTN_OK,					confirm_settings_reboot},
//	{M_CONFIRM_SETTINGS,		BTN_ESC,				confirm_settings_restore},

	{M_MAIN,					BTN_PWR_LONG,			power_long},
	{M_NAVIGATION,				BTN_PWR_LONG,			power_long},
	{M_BEACONS,					BTN_PWR_LONG,			power_long},
	{M_DEVICES,					BTN_PWR_LONG,			power_long},
	{M_NAVTO_POINTS,			BTN_PWR_LONG,			power_long},
	{M_POINTS,					BTN_PWR_LONG,			power_long},
	{M_POINTS,					BTN_ESC,				points_esc},
//	{M_ACTIONS,					BTN_UP,					actions_up},
	{M_ACTIONS,					BTN_PWR_LONG,			power_long},
	{M_ACTIONS,					BTN_OK,					actions_ok},
	{M_ACTIONS,					BTN_ESC,				actions_esc},

	{M_CONFIRM_RESTORING,		BTN_OK,					confirm_settings_restore},
    {0, 0, 0}   //end marker
};

//Defaul behaviour (non exclusive) when OK button has been pressed (move forward)
const struct
{
    uint8_t current_menu;
    uint8_t current_item;
    uint8_t next_menu;
} menu_forward_table[] = 
{
//  Current Menu                Current Item                		Next Menu
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_ALARMS,				M_SET_POINTS},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_GROUP1,				M_SET_POINTS},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_GROUP2,				M_SET_POINTS},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_GROUP3,				M_SET_POINTS},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_GROUP4,	 			M_SET_POINTS},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_GROUP5,				M_SET_POINTS},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_BCNMAN,				M_SET_POINTS},
//	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_BCNAUT,				M_SET_POINTS},
	{M_POINTS,					M_POINTS_I_ALARMS,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_GROUP1,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_GROUP2,			 		M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_GROUP3,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_GROUP4,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_GROUP5,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_BCNMAN,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_SAVED1,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_SAVED2,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_SAVED3,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_SAVED4,					M_POINTS_SUBMENU},
	{M_POINTS,					M_POINTS_I_SAVED5,					M_POINTS_SUBMENU},

	{M_SETTINGS,				M_SETTINGS_I_DEVICE,				M_SET_DEVICE_NUMBER},
	{M_SETTINGS,				M_SETTINGS_I_SPREADING_FACTOR,		M_SET_SPREADING_FACTOR},
//	{M_SETTINGS,				M_SETTINGS_I_FREQ_REGION,			M_SET_FREQ_REGION},
	{M_SETTINGS,				M_SETTINGS_I_CODING_RATE,			M_SET_CODING_RATE},
	{M_SETTINGS,				M_SETTINGS_I_FREQ_CHANNEL,			M_SET_FREQ_CHANNEL},
	{M_SETTINGS,				M_SETTINGS_I_TX_POWER,				M_SET_TX_POWER},
	{M_SETTINGS,				M_SETTINGS_I_TIMEZONE,				M_SET_TIMEZONE},
	{M_SETTINGS,				M_SETTINGS_I_TIMEOUT,				M_SET_TIMEOUT},
	{M_SETTINGS,				M_SETTINGS_I_FENCE,					M_SET_FENCE},
//	in actions_ok() implementation:
//	{M_ACTIONS,					M_ACTIONS_I_DEL_POINTS,				M_CONFIRM_ERASING},
//	{M_ACTIONS,					M_ACTIONS_I_SET_DEFAULTS,			M_CONFIRM_RESTORING},
    {0, 0, 0}   //end marker
};

//Defaul behaviour (non exclusive) when ESC button has been pressed (move backward)
const struct
{
    uint8_t current_menu;
    uint8_t next_menu;
} menu_backward_table[] = 
{
//  Current Menu                Next Menu
    {M_NAVIGATION,              M_MAIN},
	{M_BEACONS,					M_NAVIGATION},

	{M_DEVICES,                 M_MAIN},
	{M_DEVICE_SUBMENU,			M_DEVICES},
	{M_SET_POINTS,				M_DEVICE_SUBMENU},

	{M_NAVTO_POINTS,	        M_MAIN},

	{M_POINTS,	                M_MAIN},
	{M_POINTS_SUBMENU,			M_POINTS},
	{M_POINTS_SUBMENU_DEVICES,	M_POINTS},

    {M_SETTINGS,                M_CONFIRM_SETTINGS},	//M_MAIN},
	{M_CONFIRM_SETTINGS,		M_SETTINGS},
    {M_SET_DEVICE_NUMBER,       M_SETTINGS},
	{M_SET_SPREADING_FACTOR,	M_SETTINGS},
//	{M_SET_FREQ_REGION,			M_SETTINGS},
	{M_SET_CODING_RATE,			M_SETTINGS},
	{M_SET_FREQ_CHANNEL,		M_SETTINGS},
	{M_SET_TX_POWER,			M_SETTINGS},
	{M_SET_TIMEZONE,	        M_SETTINGS},
	{M_SET_TIMEOUT,				M_SETTINGS},
	{M_SET_FENCE,				M_SETTINGS},

	{M_CONFIRM_RESTORING,		M_ACTIONS},
    {0, 0}      //end marker
};

//Struct with list of menus and real-time values of current item in current menu. Last Item is needed for scroll function
//note: if current menu has no items (like INFO menu) no need to put it in structure below, because item functions (get, get last, set) automatically return 0 (which is zero item)
struct
{
    const uint8_t curent_menu;
    uint8_t cur_item;
    const uint8_t last_item;
} item_table[] = 
{
//  Current Menu                Current Item                Last Item in Current Menu
    {M_MAIN,                    M_ALL_I_FIRST,              M_MAIN_I_LAST},
	{M_DEVICE_SUBMENU,			M_ALL_I_FIRST,				M_DEVICE_SUBMENU_I_LAST},
	{M_POINTS, 	                M_ALL_I_FIRST,              M_POINTS_I_LAST},
    {M_SETTINGS,                M_ALL_I_FIRST,              M_SETTINGS_I_LAST},
	{M_ACTIONS,					M_ALL_I_FIRST,				M_ACTIONS_I_LAST},
    {0, 0, 0}   //end marker
};

//List of menus with appropriate functions to draw it (show on screen)
const struct
{
    uint8_t current;
    void (*action)(void);
} menu_draw_table[] = 
{
//  Current Menu                Draw Function
	{M_MAIN,                    draw_main},

	{M_NAVIGATION,				draw_navigation},
	{M_BEACONS,					draw_beacons},

	{M_DEVICES,                 draw_devices},
	{M_DEVICE_SUBMENU,     		draw_device_submenu},
	{M_SET_POINTS,				draw_set_points},

	{M_NAVTO_POINTS,			draw_navto_points},

	{M_POINTS,                  draw_points},
	{M_POINTS_SUBMENU,			draw_show_points},

    {M_SETTINGS,                draw_settings},
    {M_SET_DEVICE_NUMBER,		draw_set_settings},
	{M_SET_SPREADING_FACTOR,	draw_set_settings},
//	{M_SET_FREQ_REGION,			draw_set_settings},
	{M_SET_CODING_RATE,			draw_set_settings},
	{M_SET_FREQ_CHANNEL,		draw_set_settings},
	{M_SET_TX_POWER,			draw_set_settings},
    {M_SET_TIMEZONE,			draw_set_settings},
	{M_SET_TIMEOUT,				draw_set_settings},
	{M_SET_FENCE,				draw_set_settings},
	{M_CONFIRM_SETTINGS,		draw_confirm_settings},

	{M_ACTIONS,					draw_actions},
	{M_CONFIRM_RESTORING,		draw_restore_defaults},
    {0, 0}      //end mark	er
};

int8_t row;
int8_t this_device;								//device number of this device, see init_menu()
int8_t current_device;
//uint8_t navigate_to_device;							//a device number that we are navigating to right now
uint8_t current_menu;                               		//Actually Current Menu value (real-time)
uint8_t return_from_power_menu; 							//Menu to return to after exit from power menu. Power menu can be accessed from different menus, therefore we have to store menu to return.
uint8_t return_from_points_menu;
char tmp_buf[16];                                   		//temporary char buffer for screen text fragments
uint8_t flag_settings_changed = 0;							//Have settings been changed?
int8_t current_point_group = 0;		//MEMORY_POINT_FIRST;		//Current number of memory point to save
int8_t brightness = 11;
uint8_t current_mem_subpoint = 0;

//uint8_t *p_freq_region_values;
uint8_t *p_coding_rate_values;
uint8_t *p_tx_power_values;

uint16_t range = 30;
uint8_t range_ind = 0;
uint8_t range_scale[] = {1, 2, 4, 8, 16, 32, 64};

struct devices_struct **pp_devices_menu;
struct points_struct **pp_points_menu;

//struct acc_data *p_acceleration_menu;
//struct mag_data *p_magnetic_field_menu;

struct settings_struct *p_settings_menu;
struct settings_struct settings_copy_menu;

//Init and show MAIN menu
void init_menu(void)
{	//Load settings and create a local copy
	p_settings_menu = get_settings();
	settings_copy_menu = *p_settings_menu;
	this_device = p_settings_menu->device_number;
//	current_device = p_settings_menu->device_number;
//	navigate_to_device = this_device;
	current_device = this_device;
	//Load all devices
	pp_devices_menu = get_devices();
	pp_points_menu = get_points();

//	p_acceleration_menu = get_acceleration();
	//Load other
//	p_freq_region_values = get_freq_region_values();
	p_coding_rate_values = get_coding_rate_values();
	p_tx_power_values = get_tx_power_values();

	//Init start menu
    current_menu = M_MAIN;
    return_from_power_menu = M_MAIN;
    set_current_item(M_MAIN_I_NAVIGATION);
    pp_devices_menu[this_device]->display_status = 1;
}

//Check for buttons and change menu if needed
void change_menu(uint8_t button_code)
{
	int8_t point_absolute_index = 0;
	if(pp_devices_menu[this_device]->display_status) {	//if lcd is on
		//search for exclusive operation for this case
		for (uint8_t i = 0; menu_exclusive_table[i].current_menu; i++)     //until end marker
		{
			if (current_menu == menu_exclusive_table[i].current_menu &&
				button_code == menu_exclusive_table[i].button_pressed)
			{
				fillScreen(BLACK);
				menu_exclusive_table[i].execute_function();
				return;         //exit
			}
		}
		//well, there is no exclusive operations for that case, perform default action
		switch (button_code)
		{
			case BTN_UP:
				scroll_up();
				break;

			case BTN_DOWN:
				scroll_down();
				break;

			case BTN_OK:
				switch_forward();
				fillScreen(BLACK);
				break;

			case BTN_ESC:
				switch_backward();
				fillScreen(BLACK);
				break;

			case BTN_PWR:
//				lcd_off();			//GPIOB->BSRR = GPIO_BSRR_BS12;		//set to 1: lcd_display_off
				HAL_LPTIM_PWM_Stop(&hlptim1);
				pp_devices_menu[this_device]->display_status = 0;
				current_point_group = 0;								//to start save points from group 1
				break;

			default:
				break;
		}	//switch (button_code)
	}else 	//if lcd is off, display_status = 0
	{
		switch (button_code)
		{
			case BTN_PWR_LONG:
//				lcd_on();
				HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);
				pp_devices_menu[this_device]->display_status = 1;
				if(current_menu == M_NAVIGATION && pp_devices_menu[this_device]->valid_fix_flag && pp_devices_menu[this_device]->flwtrek_flag)
				{
					find_nearest_trekpoint_flag = 1;		//find_nearest_trekpoint();
					fillScreen(BLACK);
				}
				break;

			case BTN_UP:				//if short pressed
				HAL_TIM_Base_Stop(&htim17);					//if not stopped on State = TX_START
				__HAL_TIM_SET_COUNTER(&htim17, 0);
				__HAL_TIM_SET_AUTORELOAD(&htim17, 1499);		//hold for 1 second
				__HAL_TIM_CLEAR_FLAG(&htim17, TIM_SR_UIF);	//clear flag
				HAL_TIM_Base_Start_IT(&htim17);				//start with IRQ interrupt to set (current_point_group = 0) by IRQ
				if(current_point_group < 5) {
					current_point_group++;
					memory_subpoint_ind[current_point_group] = 0;		//to start from point 1
//					led_blue_on();		//current_point_group has set
				}else {
					shortBeepsBlocking(5);			//if(current_point_group == 5)
					current_point_group = 0;
//					led_blue_off();		//current_point_group has reseted
				}
				break;

			case BTN_OK:		//if long pressed && (current_point_group > 0))
				HAL_TIM_Base_Stop_IT(&htim17);				//stop before IRQ
				__HAL_TIM_CLEAR_FLAG(&htim17, TIM_SR_UIF);	//clear flag
				HAL_TIM_Base_Stop_IT(&htim17);				//todo get rid of undue "stop"
				if(!pp_devices_menu[this_device]->valid_fix_flag) {
					shortBeeps(3);				//if no valid_fix_flag when attempt to save point
					current_point_group = 0;
//					led_blue_off();		//current_point_group has reseted
					break;		//leave case BTN_OK
				}
				if(current_point_group > 0) {
					longBeepsBlocking(current_point_group);
				}else break;	//leave case BTN_OK

				do
				{
					memory_subpoint_ind[current_point_group]++;
					if(memory_subpoint_ind[current_point_group] == MEMORY_SUBPOINTS)
					{
						memory_subpoint_ind[current_point_group] = 0;								//exist_flag always == 0
						pp_points_menu[current_point_group * MEMORY_SUBPOINTS]->exist_flag = 0;		//clear subpoint 0
					}
					point_absolute_index = current_point_group * MEMORY_SUBPOINTS + memory_subpoint_ind[current_point_group];
				} while (pp_points_menu[point_absolute_index]->exist_flag);		// == 1
				if(memory_subpoint_ind[current_point_group] == 0)
				{
					current_point_group = 0;
//					led_blue_off();		//current_point_group has reseted
					break;			//longBeeps(current_point_group) and leave case BTN_OK
				}

				save_one_point(point_absolute_index);
//pp_points_menu[point_absolute_index]->exist_flag = 1;
//pp_points_menu[point_absolute_index]->latitude.as_integer = pp_devices_menu[this_device]->latitude.as_integer;
//pp_points_menu[point_absolute_index]->longitude.as_integer = pp_devices_menu[this_device]->longitude.as_integer;
//				memory_points_save();		//save to flash

				shortBeepsBlocking(memory_subpoint_ind[current_point_group]);	//number of point to save
//				memory_points_load();
//				settings_load();
				fillScreen(BLACK);
//				return_from_points_menu = current_menu;
				current_menu = M_POINTS;
				set_current_item(current_point_group);
				current_point_group = 0;
//				led_blue_off();		//current_point_group has reseted
				break;		//leave case BTN_OK

			case BTN_DOWN:				//if short pressed
				HAL_TIM_Base_Stop(&htim17);					//if not stopped on State = TX_START
				__HAL_TIM_SET_COUNTER(&htim17, 0);
				__HAL_TIM_SET_AUTORELOAD(&htim17, 1499);		//hold for 1 second
				__HAL_TIM_CLEAR_FLAG(&htim17, TIM_SR_UIF);	//clear flag
				HAL_TIM_Base_Start_IT(&htim17);				//start with IRQ interrupt to set (current_point_group = 0) by IRQ
				if(current_point_group < 11) {				//5 groups + beaconM + beaconA(RoundRobin)
					current_point_group++;
//					led_blue_on();		//current_point_group has set
				}else {
					shortBeepsBlocking(5);			//if(current_point_group == 12)
					current_point_group = 0;
//					led_blue_off();		//current_point_group has reseted
				}
				break;

			case BTN_ESC:		//if long pressed && (current_point_group > 0))
				HAL_TIM_Base_Stop_IT(&htim17);				//stop before IRQ
				__HAL_TIM_CLEAR_FLAG(&htim17, TIM_SR_UIF);	//clear flag
				HAL_TIM_Base_Stop_IT(&htim17);				//todo get rid of undue "stop"
				if(current_point_group > 0) {
//					longBeepsBlocking(current_point_group);
					shortBeeps(1);							//just for notice instead of actual long beeps
				}else break;	//leave case BTN_ESC

				clear_points_group(current_point_group);
//				for(int8_t i = 0; i < MEMORY_SUBPOINTS; i++) {	//reset one of 5 groups + beaconM + beaconA(RoundRobin)
//					point_absolute_index = current_point_group * MEMORY_SUBPOINTS + i;
//					pp_points_menu[point_absolute_index]->exist_flag = 0;
//				}
//				memory_points_save();		//save to flash
//				memory_points_load();
//				settings_load();
				fillScreen(BLACK);
//				return_from_points_menu = current_menu;
				current_menu = M_POINTS;
				set_current_item(current_point_group);
				current_point_group = 0;
//				led_blue_off();		//current_point_group has reseted
				break;		//leave case BTN_ESC

			default:
				current_point_group = 0;
//				led_blue_off();		//current_point_group has reseted
				break;
		}	//switch (button_code)
	}		//if lcd is off
}
//Scroll current menu Up
void scroll_up(void)
{
    uint8_t current = get_current_item();
    uint8_t last = get_last_item();
    
    if (current == M_ALL_I_FIRST)
    {
        set_current_item(last);
    }
    else
    {
        set_current_item(current - 1);
    }
    //draw_current_menu();
}
//Scroll current menu Down
void scroll_down(void)
{
    uint8_t current = get_current_item();
    uint8_t last = get_last_item();
    
    if (current == last)
    {
        set_current_item(M_ALL_I_FIRST);
    }
    else
    {
        set_current_item(current + 1);
    }
    //draw_current_menu();
}
//Switch menu forward by default
void switch_forward(void)
{
    for (uint8_t i = 0; menu_forward_table[i].current_menu; i++)
    {
        if (current_menu == menu_forward_table[i].current_menu &&
            get_current_item() == menu_forward_table[i].current_item)
        {
            current_menu = menu_forward_table[i].next_menu;
            break;
        }
    }
    //draw_current_menu();
}
//Switch menu backward by default
void switch_backward(void)
{
    for (uint8_t i = 0; menu_backward_table[i].current_menu; i++)
    {
        if (current_menu == menu_backward_table[i].current_menu)
        {
            set_current_item(M_ALL_I_FIRST);        //reset current item before exit
            current_menu = menu_backward_table[i].next_menu;
            break;
        }
    }
    //draw_current_menu();
}
//Get currently selected item in current menu
uint8_t get_current_item(void)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (current_menu == item_table[i].curent_menu)
        {
            return item_table[i].cur_item;
        }
    }
    return 0;       //automatically return 0 if item not found in item_table[]
}
//Get last item in current menu
uint8_t get_last_item(void)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (current_menu == item_table[i].curent_menu)
        {
            return item_table[i].last_item;
        }
    }
    return 0;       //automatically return 0 if item not found in item_table[]
}
//Set item to be current in current menu
void set_current_item(uint8_t new_value)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (current_menu == item_table[i].curent_menu)
        {
            item_table[i].cur_item = new_value;
            break;
        }
    }
}
//Reset item in any menu
void reset_current_item_in_menu(uint8_t menu)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (menu == item_table[i].curent_menu)
        {
            item_table[i].cur_item = M_ALL_I_FIRST;
            break;
        }
    }
}
//--------------------------------------------------------------
//--------------------------- DRAW -----------------------------
//Draw current menu
void draw_current_menu(void)
{
	if(pp_devices_menu[this_device]->display_status)
	{
		for (uint8_t i = 0; menu_draw_table[i].current; i++)
		{
			if (current_menu == menu_draw_table[i].current)
			{
				menu_draw_table[i].action();
				break;
			}
		}
	}
	else if(!IS_BEACON)	//if display has not lit, if IS_BEACON do nothing
	{
		for(int8_t i = 1; i < 1 + p_settings_menu->devices_on_air; i++)
		{
			if(i != this_device)
			{
				if(pp_devices_menu[i]->emergency_flag || pp_devices_menu[i]->alarm_flag || pp_devices_menu[i]->gather_flag)
				{
					fillScreen(BLACK);
					HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);
					pp_devices_menu[this_device]->display_status = 1;
//todo go to alarm point while display_status==0
					points_group_ind = 0;
					draw_navto_points();			//to draw immediately
					current_menu = M_NAVTO_POINTS;	//do not return previous menu
				}
			}
		}
	}
}
//------------------------------------------------------------------
//---------------------------MAIN MENU------------------------------
void draw_main(void)
{
	uint16_t year = (PVTbuffer[5+6]<<8) + PVTbuffer[4+6];
	uint8_t hour = PVTbuffer[14];
	row = 0;

	if(PVTbuffer[22+6] & 0x20)		//bit 5: information about UTC Date and Time of Day validity conﬁrmation is available
	{
		year = year - 2000;
		hour = hour + p_settings_menu->time_zone_hour;
		if(hour > 24) hour = hour - 24;
	}
	if(pp_devices_menu[this_device]->batt_voltage < 50)		// U < 3.2volt (!pp_devices_menu[this_device]->batt_voltage)
	{
		sprintf(&Line[row][0], "DevID:%02d Batt low!", this_device);
	}else sprintf(&Line[row][0], "DevID:%02d %d.%02d Volt", this_device, (pp_devices_menu[this_device]->batt_voltage+270)/100,
    															  (pp_devices_menu[this_device]->batt_voltage+270)%100);
	row+=1;	//1
	sprintf(&Line[row][0], "%02d/%02d/%02d  %02d:%02d:%02d", PVTbuffer[7+6], PVTbuffer[6+6], year, hour, PVTbuffer[15], PVTbuffer[16]);
	for (uint8_t k = 0; k < row+1; k++)
	{
		ST7735_WriteString(0, 1+k*11, &Line[k][0], Font_7x10, WHITE,BLACK);
	}
	row+=1;	//2
	if(nav_pvt_ram_flag)
	{
		sprintf(&Line[row][0], " %5s pDop:%2d.%02d ", fixType[pp_devices_menu[this_device]->fix_type_opt], pp_devices_menu[this_device]->p_dop/100,
																			  	  	  	  	  	  	  	  	pp_devices_menu[this_device]->p_dop%100);
		(pp_devices_menu[this_device]->valid_fix_flag)?
		ST7735_WriteString(0, 1+row*11, &Line[row][0], Font_7x10, WHITE,BLACK): ST7735_WriteString(0, 1+row*11, &Line[row][0], Font_7x10, CYAN,BLACK);
	}
	else
	{
		shortBeeps(2);			//1 beep actually every PPS
		sprintf(&Line[row][0], "GPS not configured");
		ST7735_WriteString(0, 1+row*11, &Line[row][0], Font_7x10, ORANGE,BLACK);
	}
	row+=1;	//3
	int8_t dev;
	for(dev = 1; dev < 1 + p_settings_menu->devices_on_air; dev++)
	{
		if(dev != this_device)
		{
			if(pp_devices_menu[dev]->emergency_flag)	// || (dev == p_settings_menu->devices_on_air))		//do not exceed devices_max
			{
				break;
			}
		}
		if(dev == p_settings_menu->devices_on_air)		//do not exceed devices_max and if this_device
		{
			dev = 0;
			break;
		}
	}
	if(pp_devices_menu[dev]->emergency_flag)
	{
		sprintf(&Line[row][0], "Dev%d set EMERGENCY", dev);
		ST7735_WriteString(0, 1+row*11, &Line[row][0], Font_7x10, CYAN,BLACK);
	}
	else
	{
		int8_t nods_on_the_air = (pp_devices_menu[1]->valid_fix_flag) + (pp_devices_menu[2]->valid_fix_flag) + (pp_devices_menu[3]->valid_fix_flag) +
																		(pp_devices_menu[4]->valid_fix_flag) + (pp_devices_menu[5]->valid_fix_flag);
		sprintf(&Line[row][0], "OnTheAir: %d/%d Nods", nods_on_the_air, p_settings_menu->devices_on_air);
		ST7735_WriteString(0, 1+row*11, &Line[row][0], Font_7x10, WHITE,BLACK);
	}
//	sprintf(&Line[2][1], "    MENU    ");
//	ST7735_WriteString(0, 10+2*19, &Line[2][1], Font_11x18, CYAN,BLACK);

	sprintf(&Line[2][0], " Navigation");
	sprintf(&Line[3][0], " Devices   ");
	sprintf(&Line[4][0], " Nav2points");
	sprintf(&Line[5][0], " Points    ");
	sprintf(&Line[6][0], " Settings  ");
	sprintf(&Line[7][0], " Actions   ");
	row = 2 + get_current_item();
	for (uint8_t k = 2; k < 8; k++)
	{
		if(k == row) ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, YELLOW,BLACK);
		else ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, GREEN,BLACK);
	}
	sprintf(&Line[row][0], ">");
	ST7735_WriteString(3, 10+row*19, &Line[row][0], Font_11x18, YELLOW,BLACK);
}

void main_ok(void)
{
	switch (get_current_item())
	{
		case M_MAIN_I_NAVIGATION:
//			timer4_start();					//start compass activity
			if(pp_devices_menu[this_device]->valid_fix_flag && pp_devices_menu[this_device]->flwtrek_flag)
			{
				find_nearest_trekpoint_flag = 1;		//find_nearest_trekpoint();
			}
			current_menu = M_NAVIGATION;
			break;
		case M_MAIN_I_DEVICES:
			current_menu = M_DEVICES;
			break;
		case M_MAIN_I_NAVTO_POINTS:
			current_menu = M_NAVTO_POINTS;
			break;
		case M_MAIN_I_POINTS:
			current_menu = M_POINTS;
			break;
		case M_MAIN_I_SETTINGS:
			current_menu = M_SETTINGS;
			break;
		case M_MAIN_I_ACTIONS:
			current_menu = M_ACTIONS;
			break;
	}
}
void set_brightness(void)
{
	if(brightness == 11){ brightness = 16;
	}else if(brightness == 16){ brightness = 0;
	}else brightness = 11;
//	HAL_LPTIM_PWM_Stop(&hlptim1);
	HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);		//__HAL_LPTIM_AUTORELOAD_SET(&hlptim1, brightness);
}
//--------------------------------MAIN MENU END------------------------------
//----------------------------------------------------------------------------
//--------------------------------NAVIGATION MENU--------------------------------
void draw_navigation(void)	//int8_t menu)
{
	row = 0;
	uint8_t dev = 0;
//	uint8_t *buffer = bufNode[current_device];
	ST7735_SetRotation(0);

	if(pp_devices_menu[this_device]->gps_speed > GPS_SPEED_THRS)
	{
		sprintf(&Line[row][4], "%2dkmh", pp_devices_menu[this_device]->gps_speed);
		ST7735_WriteString(37, row*19, &Line[row][4], Font_11x18, GREEN,BLACK);
	 	(heading_deg < 100)? (sprintf(&Line[row+=1][4], "%3d%% ", heading_deg)): (sprintf(&Line[row+=1][4], " %3d%%", heading_deg));	//just shift->
	 	ST7735_WriteString(37, (row)*19, &Line[row][4], Font_11x18, YELLOW,BLACK);
	}else if(is_north_ready())
	{
		sprintf(&Line[row][4], " Magn");	//"Axis%3d ", (p_settings_menu->accel_max.as_integer - p_acceleration_menu->acc_z.as_integer));
		ST7735_WriteString(37, row*19, &Line[row][4], Font_11x18, CYANB,BLACK);
	 	(heading_deg < 100)? (sprintf(&Line[row+=1][4], "%3d%% ", heading_deg)): (sprintf(&Line[row+=1][4], " %3d%%", heading_deg));	//just shift->
	 	ST7735_WriteString(37, (row)*19, &Line[row][4], Font_11x18, GREEN,BLACK);
	}else
	{
		sprintf(&Line[row][4], " TURN");
		ST7735_WriteString(37, row*19, &Line[row][4], Font_11x18, RED,BLACK);
		sprintf(&Line[row+=1][4], " ARND");
		ST7735_WriteString(37, (row)*19, &Line[row][4], Font_11x18, RED,BLACK);
	}

	if(is_north_ready())
	{
		(heading_rad < 0)? (north_rad = -heading_rad): (north_rad = 2*M_PI - heading_rad);
		drawArrow(63, 97, 57, north_rad_old, 28, BLACK, BLACK);
		drawArrow(63, 97, 57, north_rad, 28, CYANB, RED);
		north_rad_old = north_rad;
	}else drawArrow(63, 97, 57, north_rad_old, 28, BLACK, BLACK);

	for(dev = 1; dev < (p_settings_menu->devices_on_air + 1); dev++)
	{
		if(dev != this_device)
		{
			if(pp_devices_menu[dev]->valid_fix_flag) break;
//			if(dev == p_settings_menu->devices_on_air) break;
		}
		if(dev == p_settings_menu->devices_on_air)
		{
			dev = 0;		//if no "valid_fix_flag" received
			break;			//do not exceed "devices_on_air" on exit
		}
	}
//	(this_device == 1)? (dev = 2): (dev = 1);
	pp_devices_menu[dev]->beacon_flag? sprintf(&Line[0][0], "Bcn:%d", dev): sprintf(&Line[0][0], "Dev:%d", dev);	//if is beacon
//		(buffer[0] >> 7)? sprintf(&Line[0][0], "Bcn:%d", dev): sprintf(&Line[0][0], "Dev:%d", dev);	//if is beacon
	//show azimuth and distance to remote
		azimuth_relative_deg = azimuth_deg_signed[dev] - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
		sprintf(&Line[1][0], "%4d%%", azimuth_relative_deg);
		sprintf(&Line[2][0], "%4dm", ((uint16_t)distance[dev] & 0x1FFF));
		sprintf(&Line[3][0], "%3ddB", pp_devices_menu[dev]->rssi);		//(int8_t)buffer[BUFFER_AIR_SIZE]);
//		if((buffer[14] & 0x0F) == 0)
//		{
//			sprintf(&Line[4][0], "low");		//< 2.7 volt
//		} else sprintf(&Line[4][0], "%d.%dV", ((buffer[14] & 0x0F)+27)/10, ((buffer[14] & 0x0F)+27)%10);
		for (uint8_t k = 0; k < 4; k++) {
			if(pp_devices_menu[dev]->valid_fix_flag) ST7735_WriteString(0, k*11, &Line[k][0], Font_7x10, YELLOW,BLACK);		//if remote fix valid (validFixFlag[dev])
			else ST7735_WriteString(0, k*11, &Line[k][0], Font_7x10, MAGENTA,BLACK);
		}

	for(dev = p_settings_menu->devices_on_air; dev > 0; dev--)
	{
		if(dev != this_device)
		{
			if(pp_devices_menu[dev]->valid_fix_flag) break;
//			if(dev == 1) break;	"dev = 0" if no "valid_fix_flag" received
		}
	}
//	(this_device == 3)? (dev = 2): (dev = 3);
	pp_devices_menu[dev]->beacon_flag? sprintf(&Line[0][13], "Bcn:%d", dev): sprintf(&Line[0][13], "Dev:%d", dev);	//if is beacon
//		(buffer[0] >> 7)? sprintf(&Line[0][13], "Bcn:%d", dev): sprintf(&Line[0][13], "Dev:%d", dev);	//if is beacon
	//show azimuth and distance to remote
		azimuth_relative_deg = azimuth_deg_signed[dev] - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;

		sprintf(&Line[1][13], "%4d%%", azimuth_relative_deg);
		sprintf(&Line[2][13], "%4dm", ((uint16_t)distance[dev] & 0x1FFF));
		sprintf(&Line[3][13], "%3ddB", pp_devices_menu[dev]->rssi);		//(int8_t)buffer[BUFFER_AIR_SIZE]);
//		if((buffer[14] & 0x0F) == 0)
//		{
//			sprintf(&Line[4][13], " low");		//< 2.7 volt
//		} else sprintf(&Line[4][13], " %d.%dV", ((buffer[14] & 0x0F)+27)/10, ((buffer[14] & 0x0F)+27)%10);
		for (uint8_t k = 0; k < 4; k++) {
			if(pp_devices_menu[dev]->valid_fix_flag) ST7735_WriteString(91, k*11, &Line[k][13], Font_7x10, YELLOW,BLACK);		//if remote fix valid (validFixFlag[dev])
			else ST7735_WriteString(91, k*11, &Line[k][13], Font_7x10, MAGENTA,BLACK);
		}

	if(pp_devices_menu[this_device]->valid_fix_flag && pp_devices_menu[this_device]->flwtrek_flag)
	{
		manage_trekpoints(range_ind);
	}

	for(uint8_t i = 0; i < (p_settings_menu->devices_on_air + 1); i++) {						//devises on the air = 3
		if(i == this_device) {										//if this device
			drawPosition(63, 97, 0, 0, 7, i, GREEN);
		}else if (pp_devices_menu[i]->valid_fix_flag) {									//if remote fix valid
			scaled_dist = ((int16_t)distance[i] & 0x1FFF)*2  / range_scale[range_ind];
			azimuth_relative_rad = azimuth_rad[i] - heading_rad;
			erasePosition(63, 97, distance_old[i], azimuth_relative_rad_old[i], 8);
			if(scaled_dist > range * 2)
			{
				scaled_dist = range * 2;
				drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, i, RED);
			}else drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, i, YELLOW);
			distance_old[i] = scaled_dist;
			azimuth_relative_rad_old[i] = azimuth_relative_rad;
		}else if(distance_old[i] > 3) erasePosition(63, 97, distance_old[i], azimuth_relative_rad_old[i], 8);	//erase or change CYAN to MAGENTA
	}		//if(distance_old[i] > 3) -> don't erase this device (yellow number)

	drawCircle(63, 97, 60, WHITE);
	row = 13;
	sprintf(&Line[row][0], "%d.%02dV", (pp_devices_menu[this_device]->batt_voltage+270)/100,
		  	   	   	   	   	    		 (pp_devices_menu[this_device]->batt_voltage+270)%100);
	ST7735_WriteString(0, 7+row*11, &Line[row][0], Font_7x10, GREEN,BLACK);
	sprintf(&Line[row][13], "%4dm", range * range_scale[range_ind]);
	ST7735_WriteString(92, 7+row*11, &Line[row][13], Font_7x10, WHITE,BLACK);
//	GPIOB->BSRR = GPIO_BSRR_BR3;	//blue led off DRAW MENU END
}
void draw_beacons(void)
{
	if(current_device == this_device) current_device++;
	if(current_device > p_settings_menu->devices_on_air) current_device = 1;

		row = 0;
		uint8_t beacons_group_start = (MEMORY_POINT_GROUPS + current_device - 1) * MEMORY_SUBPOINTS;	//points_group_ind * MEMORY_SUBPOINTS;
//	uint8_t *buffer = bufNode[current_device];
		pp_points_menu[beacons_group_start]->exist_flag = 0;			//clear sub-point 0
		for(uint8_t j = 1; j < MEMORY_SUBPOINTS; j++)
		{
			if(pp_points_menu[beacons_group_start + j]->exist_flag == 1)
			{
			pp_points_menu[beacons_group_start]->exist_flag++;			//save to sub-point 0
			}
		}

		ST7735_SetRotation(0);
//	buffer = bufNode[dev];
		pp_devices_menu[current_device]->beacon_flag? sprintf(&Line[row][0], "tBcn%d", current_device): sprintf(&Line[row][0], "Dev%d:", current_device);	//if is beacon or device
		ST7735_WriteString(0, row*18, &Line[row][0], Font_11x18, YELLOW,BLACK);
		if(pp_devices_menu[current_device]->beacon_traced)
		{
			(pp_devices_menu[current_device]->gps_speed)?
					sprintf(&Line[row][5], "%2dkm/h", pp_devices_menu[current_device]->gps_speed):
					sprintf(&Line[row][5], "Traced");		//"Trcd%2d", pp_devices_menu[current_device]->beacon_traced);

			(pp_devices_menu[current_device]->gps_speed)?
					ST7735_WriteString(60, row*18, &Line[row][5], Font_11x18, CYAN,BLACK):
					ST7735_WriteString(60, row*18, &Line[row][5], Font_11x18, GREEN,BLACK);
		}else if(pp_devices_menu[current_device]->beacon_lost)
		{
			sprintf(&Line[row][5], "!LOST!");
			ST7735_WriteString(60, row*18, &Line[row][5], Font_11x18, RED,BLACK);
		}else if(pp_points_menu[beacons_group_start]->exist_flag)
		{
			sprintf(&Line[row][5], "%dsaved", pp_points_menu[beacons_group_start]->exist_flag);	//amount of saved sub-points);
			ST7735_WriteString(60, row*18, &Line[row][5], Font_11x18, CYAN,BLACK);
		}else
		{
			sprintf(&Line[row][5], "absent");
			ST7735_WriteString(60, row*18, &Line[row][5], Font_11x18, MAGENTA,BLACK);
		}

		if(is_north_ready())
		{
//show azimuth and distance to remote
			if(pp_devices_menu[current_device]->valid_fix_flag)
			{
				azimuth_relative_deg = azimuth_deg_signed[current_device] - heading_deg;
			}else
			{
				azimuth_relative_deg = pp_points_menu[beacons_group_start + 1]->azimuth_deg_signed - heading_deg;
				distance[current_device] = (pp_points_menu[beacons_group_start + 1]->distance);
			}

			if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
			if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
//		sprintf(&Line[++row][0], "%4d%% %4dm", azimuth_relative_deg, ((uint16_t)distance[dev] & 0x1FFF))
			sprintf(&Line[++row][0], "%4d%%%5dm", azimuth_relative_deg, (uint16_t)distance[current_device]);
			if(pp_devices_menu[current_device]->valid_fix_flag) ST7735_WriteString(0, row*18, &Line[row][0], Font_11x18, YELLOW,BLACK);		//if remote fix valid (validFixFlag[dev])
			else ST7735_WriteString(0, row*18, &Line[row][0], Font_11x18, MAGENTA,BLACK);
//draw magnet arrow
			(heading_rad < 0)? (north_rad = -heading_rad): (north_rad = 2*M_PI - heading_rad);
			drawArrow(63, 97, 57, north_rad_old, 28, BLACK, BLACK);
			drawArrow(63, 97, 57, north_rad, 28, CYANB, RED);
			north_rad_old = north_rad;
		}else
		{
			sprintf(&Line[++row][0], "TURN AROUND");
			ST7735_WriteString(0, row*18, &Line[row][0], Font_11x18, RED,BLACK);
//erase magnet arrow
			drawArrow(63, 97, 57, north_rad_old, 28, BLACK, BLACK);
//		sprintf(&Line[3][4], "TURN");
//		ST7735_WriteString(42, 3*18, &Line[3][4], Font_11x18, RED,BLACK);
//		sprintf(&Line[4][3], "AROUND");
//		ST7735_WriteString(31, 4*18, &Line[4][3], Font_11x18, RED,BLACK);
		}

		if(pp_devices_menu[current_device]->batt_voltage < 32)
		{
			sprintf(&Line[++row][0], " LOW");		//< 3.2 volt
		} else sprintf(&Line[++row][0], "%d.%dV", pp_devices_menu[current_device]->batt_voltage/10, pp_devices_menu[current_device]->batt_voltage%10);
		if(pp_devices_menu[current_device]->valid_fix_flag) ST7735_WriteString(0, row*18, &Line[row][0], Font_7x10, CYAN,BLACK);
		else ST7735_WriteString(0, row*18, &Line[row][0], Font_7x10, MAGENTA,BLACK);

		sprintf(&Line[row][12], "%4ddB", pp_devices_menu[current_device]->rssi);
		if(pp_devices_menu[current_device]->valid_fix_flag) ST7735_WriteString(85, row*18, &Line[row][12], Font_7x10, CYAN,BLACK);		//if remote fix valid (validFixFlag[dev])
		else ST7735_WriteString(85, row*18, &Line[row][12], Font_7x10, MAGENTA,BLACK);

		row = 13;
		sprintf(&Line[row][0], "%d.%02dV", (pp_devices_menu[this_device]->batt_voltage+270)/100,
  	   	    		 	 	 	 	 (pp_devices_menu[this_device]->batt_voltage+270)%100);
										//(GetBatteryLevel()+270)/100, (GetBatteryLevel()+270)%100);
		ST7735_WriteString(0, 7+row*11, &Line[row][0], Font_7x10, GREEN,BLACK);
		sprintf(&Line[row][13], "%4dm", range * range_scale[range_ind]);
		ST7735_WriteString(92, 7+row*11, &Line[row][13], Font_7x10, WHITE,BLACK);

		drawCircle(63, 97, 60, WHITE);

		for(uint8_t i = 1; i < MEMORY_SUBPOINTS; i++)
		{
			if(pp_points_menu[beacons_group_start + i]->exist_flag == 1) {
				if(pp_devices_menu[this_device]->valid_fix_flag) calc_point_position(beacons_group_start + i); //host device valid GPS fix pDop and accuracy
			}
		}
		for(uint8_t i = 1; i < MEMORY_SUBPOINTS; i++)
		{
			if(pp_points_menu[beacons_group_start + i]->exist_flag == 1) {
				scaled_dist = (uint16_t)(pp_points_menu[beacons_group_start + i]->distance)*2  / range_scale[range_ind];
				azimuth_relative_rad = pp_points_menu[beacons_group_start + i]->azimuth_rad - heading_rad;
				erasePosition(63, 97, distance_old[i], azimuth_relative_rad_old[i], 8);
				if(scaled_dist > range * 2)
				{
					scaled_dist = range * 2;
					drawTrace(63, 97, scaled_dist, azimuth_relative_rad, (8 - i), RED);
				}else drawTrace(63, 97, scaled_dist, azimuth_relative_rad, (8 - i), CYAN);
				distance_old[i] = scaled_dist;
				azimuth_relative_rad_old[i] = azimuth_relative_rad;
			}//else if(i == this_device) drawPosition(63, 97, 0, 0, 7, i, YELLOW);
		}

			if (pp_devices_menu[current_device]->valid_fix_flag) {									//if remote fix valid
				scaled_dist = ((int16_t)distance[current_device] & 0x1FFF)*2  / range_scale[range_ind];
				azimuth_relative_rad = azimuth_rad[current_device] - heading_rad;
				erasePosition(63, 97, distance_old[current_device], azimuth_relative_rad_old[current_device], 8);
				if(scaled_dist > range * 2)
				{
					scaled_dist = range * 2;
					drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, current_device, RED);
				}else drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, current_device, YELLOW);
				distance_old[current_device] = scaled_dist;
				azimuth_relative_rad_old[current_device] = azimuth_relative_rad;
			}else if(distance_old[current_device] > 3) erasePosition(63, 97, distance_old[current_device], azimuth_relative_rad_old[current_device], 8);	//erase or change CYAN to MAGENTA
//		}		//if(distance_old[i] > 3) -> don't erase this device (yellow number)
		drawPosition(63, 97, 0, 0, 7, this_device, GREEN);
//	GPIOB->BSRR = GPIO_BSRR_BR3;	//blue led off DRAW MENU END
}
//--------------------------------NAVIGATION MENU SET--------------------------------
void navigation_up(void) {
	if(range_ind == 6) {
		range_ind = 0;
	}else {
		range_ind++;
	}
}
void navigation_down(void) {
	if(range_ind == 0) {
		range_ind = 6;
	}else {
		range_ind--;
	}
}
void navigation_ok(void) {		//other points on loop
	current_menu = M_BEACONS;
}
//void navigation_esc(void) {
////	timer4_stop();	//stop compass activity
//	current_menu = M_MAIN;
//}
void beacons_ok(void)
{
	if(current_device == p_settings_menu->devices_on_air) current_device = 1;
	else current_device++;
//	distance_old[i] = 0;
//	draw_beacons();
}
void beacons_esc(void)
{
//	if(current_device == 1) current_device = p_settings_menu->devices_on_air;
//	else current_device--;
//	distance_old[i] = 0;
	if(pp_devices_menu[this_device]->valid_fix_flag && pp_devices_menu[this_device]->flwtrek_flag) find_nearest_trekpoint_flag = 1;
	current_menu = M_NAVIGATION;
}
//--------------------------------NAVIGATION MENU END--------------------------------
//-----------------------------------------------------------------------------------
//--------------------------------DEVICES MENU----------------------------
void draw_this_device(void)
{
	uint16_t year = (PVTbuffer[5+6]<<8) + PVTbuffer[4+6];
	uint8_t hour = PVTbuffer[14];

	if(PVTbuffer[22+6] & 0x20)		//bit 5: information about UTC Date and Time of Day validity conﬁrmation is available
	{
		year = year - 2000;
		hour = hour + p_settings_menu->time_zone_hour;
		if(hour > 24) hour = hour - 24;
	}
	ST7735_SetRotation(0);
		sprintf(&Line[0][0], " Device: %02d ", this_device);
			if(pp_devices_menu[this_device]->valid_fix_flag) ST7735_WriteString(0, 0, &Line[0][0], Font_11x18, YELLOW,BLACK);		//if fix valid
			else ST7735_WriteString(0, 0, &Line[0][0], Font_11x18, MAGENTA,BLACK);
//		sprintf(&Line[2][0], "LoRa 433MHz SF=%02d", p_settings_menu->spreading_factor);
		sprintf(&Line[2][0], "LoRa%3d.%03dMHzSF%02d", (433000 + 50 + p_settings_menu->freq_channel * 25)/1000,
													  (433000 + 50 + p_settings_menu->freq_channel * 25)%1000,
												      p_settings_menu->spreading_factor);
		sprintf(&Line[3][0], "TxPWR %2ddBm CR=4/%d", p_tx_power_values[p_settings_menu->tx_power_opt], p_coding_rate_values[p_settings_menu->coding_rate_opt]);
		sprintf(&Line[4][0], "CPU:%02d%%C;%d.%02d Volt",pp_devices_menu[this_device]->core_temperature,
													 (pp_devices_menu[this_device]->core_voltage+270)/100,
				  	  	  	  	  	  	  	  	  	 (pp_devices_menu[this_device]->core_voltage+270)%100);
		sprintf(&Line[5][0], "Battery: %d.%02d Volt", (pp_devices_menu[this_device]->batt_voltage+270)/100,
 	  	  	  	  	  	  	  	   	   	   	   	    (pp_devices_menu[this_device]->batt_voltage+270)%100);
		for (uint8_t k = 2; k < 6; k++)
		{
			//ST7735_WriteString(0, k*9, Line[k], Font_6x8, GREEN,BLACK);
			ST7735_WriteString(0, k*11, &Line[k][0], Font_7x10, YELLOW,BLACK);
		}

		sprintf(&Line[6][0], " %5s pDop:%2d.%02d ", fixType[pp_devices_menu[current_device]->fix_type_opt],
				pp_devices_menu[current_device]->p_dop/100, pp_devices_menu[current_device]->p_dop%100);
//				[PVTbuffer[20+6]], ((PVTbuffer[77+6]<<8)+PVTbuffer[76+6])/100, ((PVTbuffer[77+6]<<8)+PVTbuffer[76+6])%100);
		sprintf(&Line[7][0], "%02d/%02d/%02d  %02d:%02d:%02d", PVTbuffer[7+6], PVTbuffer[6+6], year, hour, PVTbuffer[15], PVTbuffer[16]);

		sprintf(&Line[8][0], "Latit : %ld", pp_devices_menu[this_device]->latitude.as_integer);	//((int32_t)(PVTbuffer[37]<<24)+(PVTbuffer[36]<<16)+(PVTbuffer[35]<<8)+PVTbuffer[34]));
		sprintf(&Line[9][0], "Longit: %ld", pp_devices_menu[this_device]->longitude.as_integer);	//((int32_t)(PVTbuffer[33]<<24)+(PVTbuffer[32]<<16)+(PVTbuffer[31]<<8)+PVTbuffer[30]));
		sprintf(&Line[10][0], "Beep Flag:");//  %d", main_flags.beeper_flag_received);
//		sprintf(&Line[10][0], "HeightEli:  %04dm", ((PVTbuffer[35+6]<<24)+(PVTbuffer[34+6]<<16)+(PVTbuffer[33+6]<<8)+PVTbuffer[32+6])/1000);
		sprintf(&Line[11][0], "HeightMSL:  %04dm", ((PVTbuffer[39+6]<<24)+(PVTbuffer[38+6]<<16)+(PVTbuffer[37+6]<<8)+PVTbuffer[36+6])/1000);
		sprintf(&Line[12][0], "Speed:   %03d km/h", pp_devices_menu[this_device]->gps_speed);

		if(pp_devices_menu[this_device]->gps_speed > GPS_SPEED_THRS) {
				sprintf(&Line[13][0], "HeadingGPS:  %03d%% ", pp_devices_menu[this_device]->gps_heading);
		}else 	sprintf(&Line[13][0], "HeadingMagn: %03d%% ", heading_deg);

//		GPIOB->BSRR = GPIO_BSRR_BR3;	//blue led off DRAW MENU END SPI start
		for (uint8_t k = 6; k < 14; k++)
		{
			//ST7735_WriteString(0, k*9, Line[k], Font_6x8, GREEN,BLACK);
			if(pp_devices_menu[current_device]->valid_fix_flag) ST7735_WriteString(0, 4+k*11, &Line[k][0], Font_7x10, GREEN,BLACK);	//(PVTbuffer[21+6] & 0x01)
			else ST7735_WriteString(0, 4+k*11, &Line[k][0], Font_7x10, MAGENTA,BLACK);
		}
//		GPIOB->BSRR = GPIO_BSRR_BS3;	//blue led on SPI stop
}

void draw_devices(void)	//int8_t menu)
{
	if(current_device == this_device) draw_this_device();
	else
	{
		if(pp_devices_menu[current_device]->device_num == current_device)		//data has received
		{
			if(pp_devices_menu[current_device]->beacon_flag)		//(buffer[0] >> 7)		//if is beacon
			{
				sprintf(&Line[0][0], "tBcn%d data:", pp_devices_menu[current_device]->device_num);			//(buffer[0] & 0x03));
			}else sprintf(&Line[0][0], " Dev%d data:", pp_devices_menu[current_device]->device_num);		//(buffer[0] & 0x03));
		}
		else sprintf(&Line[0][0], " NoDevice %d ", current_device);
		ST7735_SetRotation(0);
		ST7735_WriteString(0, 0, &Line[0][0], Font_11x18, YELLOW,BLACK);

		//fixType only 2 bits used to transmit, pDop/10 (0...25.5)	buffer[15]/10, buffer[15]%10);
		sprintf(&Line[2][0], " %5s pDop:%2d.%d", fixType[pp_devices_menu[current_device]->fix_type_opt],
				pp_devices_menu[current_device]->p_dop/10, pp_devices_menu[current_device]->p_dop%10);

//	sprintf(&Line[3][0], "Date../09 %02d:%02d:%02d", pp_devices_menu[current_device]->time_hours,
//			pp_devices_menu[current_device]->time_minutes, pp_devices_menu[current_device]->time_seconds);
		sprintf(&Line[3][0], "Distance:%5d m", ((uint16_t)distance[current_device] & 0xFFFF));
		azimuth_relative_deg = azimuth_deg_signed[current_device] - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
		sprintf(&Line[4][0], "AzRelative: %03d%%", azimuth_relative_deg);

		sprintf(&Line[5][0], "Speed:   %3d km/h", pp_devices_menu[current_device]->gps_speed);
		if(pp_devices_menu[current_device]->gps_speed)
		{
			sprintf(&Line[6][0], "HeadingGPS:  %03d%% ", pp_devices_menu[current_device]->gps_heading);
		}else 	sprintf(&Line[6][0], "Device%d not moving", current_device);

//	sprintf(&Line[6][0], "Azimuth_s : %03d%%", azimuth_deg_signed[current_device]);
//	sprintf(&Line[7][0], "Azimuth_u : %03d%%", azimuth_deg_unsigned[current_device]);
		sprintf(&Line[7][0], "                  ");

		if(pp_devices_menu[current_device]->batt_voltage < 32)
		{
			sprintf(&Line[8][0], "Battery :  low    ");		//<=3.2 volt
		} else sprintf(&Line[8][0], "Battery: %d.%d Volt ", pp_devices_menu[current_device]->batt_voltage/10, pp_devices_menu[current_device]->batt_voltage%10);

		sprintf(&Line[9][0], "RSSI: %ddBm", pp_devices_menu[current_device]->rssi);
		sprintf(&Line[10][0], " SNR: %02ddB", pp_devices_menu[current_device]->snr);

		(p_settings_menu->spreading_factor == 12)? sprintf(&Line[11][0], "                  "):
		sprintf(&Line[11][0], "RX%d to TX%d: %4dmS", pp_devices_menu[current_device]->device_num, this_device, endRX_2_TX);

		sprintf(&Line[12][0], "Latit : %ld", pp_devices_menu[current_device]->latitude.as_integer);
		sprintf(&Line[13][0], "Longit: %ld", pp_devices_menu[current_device]->longitude.as_integer);

		for (uint8_t k = 2; k < 14; k++)
		{
			if(pp_devices_menu[current_device]->valid_fix_flag) ST7735_WriteString(0, 4+k*11, &Line[k][0], Font_7x10, GREEN,BLACK);	//if remote fix valid (validFixFlag[current_device]),((buffer[14] & 0x10) >> 4)
			else ST7735_WriteString(0, 4+k*11, &Line[k][0], Font_7x10, MAGENTA,BLACK);
		}
	}// end of else (current_device != this_device)
}
//-----------------------------DEVICES MENU SET------------------------------------
void scroll_devices_up(void) {
        if (current_device == p_settings_menu->devices_on_air)
        {
        	current_device = 1;
        }else {
        	current_device++;
        }
        draw_devices();
}
void scroll_devices_down(void) {
        if (current_device == 1)
        {
        	current_device = p_settings_menu->devices_on_air;
        }else {
        	current_device--;
        }
        draw_devices();
}
void devices_ok(void) {
	current_menu = M_DEVICE_SUBMENU;
}
void devices_esc(void) {
//	timer4_start();	//stop compass activity
	current_menu = M_MAIN;
}
//---------------------------------DEVICES MENU END---------------------------------
//----------------------------------------------------------------------
//--------------------------------NAV TO POINTS MENU--------------------------------
int8_t ind = 0;
void draw_navto_points(void)
{
	row = 0;
	uint8_t sub_point_ind = 0;
	uint8_t sub_point[] = {0, 0, 0, 0, 0, 0, 0};		//1...7
	uint8_t points_group_start = points_group_ind * MEMORY_SUBPOINTS;

		for(uint8_t i = 1; i < MEMORY_SUBPOINTS; i++)
		{
			if(pp_points_menu[points_group_start + i]->exist_flag == 1) {
				if(pp_devices_menu[this_device]->valid_fix_flag) calc_point_position(points_group_start + i); //host device valid GPS fix pDop and accuracy
				sub_point[sub_point_ind] = i;						//save number of saved sub-point
//				if(sub_point_ind++ > 9) i = MEMORY_SUBPOINTS;			//restrict to 9 for a while
				sub_point_ind++;	//amount of saved sub points
			}
		}

	ST7735_SetRotation(0);
	if(sub_point[ind]) {
		azimuth_relative_deg = pp_points_menu[points_group_start+sub_point[ind]]->azimuth_deg_signed - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
	}else azimuth_relative_deg = 0;
	sprintf(&Line[0][0], "%1d:%5dm%3d%%", sub_point[ind], (uint16_t)(pp_points_menu[points_group_start+sub_point[ind]]->distance), azimuth_relative_deg);
	ind++;
	if(sub_point[ind]) {
		azimuth_relative_deg = pp_points_menu[points_group_start+sub_point[ind]]->azimuth_deg_signed - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
	}else azimuth_relative_deg = 0;
	sprintf(&Line[1][0], "%1d:%5dm%3d%%", sub_point[ind], (uint16_t)(pp_points_menu[points_group_start+sub_point[ind]]->distance), azimuth_relative_deg);
	ind++;
	if(sub_point[ind]) {
		azimuth_relative_deg = pp_points_menu[points_group_start+sub_point[ind]]->azimuth_deg_signed - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
	}else azimuth_relative_deg = 0;
	sprintf(&Line[2][0], "%1d:%5dm%3d%%", sub_point[ind], (uint16_t)(pp_points_menu[points_group_start+sub_point[ind]]->distance), azimuth_relative_deg);
	((ind + 2) > sub_point_ind)? ind = 0: ind--;
//	sprintf(&Line[3][0], "%4s:%d", get_points_group_short(points_group_ind), sub_point_ind);
	sprintf(&Line[3][0], "%4s", get_points_group_short(points_group_ind));
	for (row = 0; row < 3; row++)
	{
		ST7735_WriteString(0, row*12, &Line[row][0], Font_7x10, GREEN,BLACK);
	}
	ST7735_WriteString(3, 3*12, &Line[3][0], Font_7x10, CYAN,BLACK);

	row = 0;
	if(pp_devices_menu[this_device]->gps_speed > GPS_SPEED_THRS)
	{
		sprintf(&Line[row][8], "%2d/h", pp_devices_menu[this_device]->gps_speed);
		ST7735_WriteString(83, row*19, &Line[row][8], Font_11x18, GREEN,BLACK);
		(heading_deg > -99)? (sprintf(&Line[row+=1][8], "%3d%%", heading_deg)): (sprintf(&Line[row+=1][8], "%3d", heading_deg));
		ST7735_WriteString(83, (row)*19, &Line[row][8], Font_11x18, YELLOW,BLACK);
	}else if(is_north_ready())
	{
		sprintf(&Line[row][8], "Magn");		//"Axis%3d ", (p_settings_menu->accel_max.as_integer - p_acceleration_menu->acc_z.as_integer));
		ST7735_WriteString(83, row*19, &Line[row][8], Font_11x18, CYANB,BLACK);
		(heading_deg > -99)? (sprintf(&Line[row+=1][8], "%3d%%", heading_deg)): (sprintf(&Line[row+=1][8], "%3d", heading_deg));
		ST7735_WriteString(83, (row)*19, &Line[row][8], Font_11x18, GREEN,BLACK);
	}else
	{
		sprintf(&Line[row][8], "TURN");
		ST7735_WriteString(83, row*19, &Line[row][8], Font_11x18, RED,BLACK);
		sprintf(&Line[row+=1][8], "ARND");
		ST7735_WriteString(83, (row)*19, &Line[row][8], Font_11x18, RED,BLACK);
	}

	if(is_north_ready())
	{
		(heading_rad < 0)? (north_rad = -heading_rad): (north_rad = 2*M_PI - heading_rad);
		drawArrow(63, 97, 57, north_rad_old, 28, BLACK, BLACK);
		drawArrow(63, 97, 57, north_rad, 28, CYANB, RED);
		north_rad_old = north_rad;
	}else drawArrow(63, 97, 57, north_rad_old, 28, BLACK, BLACK);

	row = 13;
	sprintf(&Line[row][0], "%d.%02dV", (pp_devices_menu[this_device]->batt_voltage+270)/100,
  	   	    		 	 	 	 	 (pp_devices_menu[this_device]->batt_voltage+270)%100);
										//(GetBatteryLevel()+270)/100, (GetBatteryLevel()+270)%100);
	ST7735_WriteString(0, 7+row*11, &Line[row][0], Font_7x10, GREEN,BLACK);
	sprintf(&Line[row][13], "%4dm", range * range_scale[range_ind]);
	ST7735_WriteString(92, 7+row*11, &Line[row][13], Font_7x10, WHITE,BLACK);

	drawCircle(63, 97, 60, WHITE);

	for(uint8_t i = 1; i < MEMORY_SUBPOINTS; i++)
	{
		if(pp_points_menu[points_group_start + i]->exist_flag == 1) {
			scaled_dist = ((uint16_t)(pp_points_menu[points_group_start + i]->distance & 0x1FFF))*2  / range_scale[range_ind];
			azimuth_relative_rad = pp_points_menu[points_group_start + i]->azimuth_rad - heading_rad;
			erasePosition(63, 97, distance_old[i], azimuth_relative_rad_old[i], 8);
			if(scaled_dist > range * 2)
			{
				scaled_dist = range * 2;
				drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, i, RED);
			}else drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, i, YELLOW);
			distance_old[i] = scaled_dist;
			azimuth_relative_rad_old[i] = azimuth_relative_rad;
		}//else if(i == this_device) drawPosition(63, 97, 0, 0, 7, i, YELLOW);
	}
	drawPosition(63, 97, 0, 0, 7, this_device, GREEN);
//	GPIOB->BSRR = GPIO_BSRR_BR3;	//blue led off DRAW MENU END
}
//------------------------------NAV TO POINTS MENU SET----------------------------
void navto_points_ok(void) {		//other points on loop
	if (points_group_ind == (MEMORY_POINT_GROUPS - 1)) points_group_ind = 0;
    else points_group_ind++;
}
void navto_points_esc(void)
{
	for(uint8_t i = 0; i < MEMORY_SUBPOINTS; i++) {
		distance_old[i] = 0;
	}		// reset saved points values
	current_menu = M_MAIN;			//M_NAVIGATION;
}
//------------------------------NAV TO POINTS MENU END----------------------------
//----------------------------------------------------------------------
//----------------------------------POINTS MENU--------------------------------
void draw_points(void)
{
	for(uint8_t i = 0; i < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); i++)							//calculate saved memory points
		{
			pp_points_menu[i * MEMORY_SUBPOINTS]->exist_flag = 0;			//clear sub-point 0
			for(uint8_t j = 1; j < MEMORY_SUBPOINTS; j++)
			{
				if(pp_points_menu[i * MEMORY_SUBPOINTS + j]->exist_flag == 1) {
					pp_points_menu[i * MEMORY_SUBPOINTS]->exist_flag++;	//save to sub-point 0: 0, 28, 55, 83, 111, ...
				}
			}
		}
	ST7735_SetRotation(0);

	sprintf(&Line[0][0], "N%%  POINTS SAVED    ");
	ST7735_WriteString(0, 0, &Line[0][0], Font_7x10, CYAN,BLACK);

	row = get_current_item();
	for (uint8_t k = 1; k < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); k++)
	{
		sprintf(&Line[k][0], "%2d", k);
		ST7735_WriteString(0, 14+k*12, &Line[k][0], Font_7x10, CYAN,BLACK);
	}
	for (uint8_t k = 0; k < (MEMORY_POINT_GROUPS + BEACON_POINT_GROUPS); k++)		//draw points groups
	{
		sprintf(&Line[k][0], " %7s   %1d", get_points_group_name(k), pp_points_menu[k * MEMORY_SUBPOINTS]->exist_flag);	//amount of saved sub-points
		if(k == row) ST7735_WriteString(21, 14+k*12, &Line[k][0], Font_7x10, YELLOW,BLACK);		//active points group
		else ST7735_WriteString(21, 14+k*12, &Line[k][0], Font_7x10, GREEN,BLACK);				//other points groups
	}
	sprintf(&Line[row][0], ">");
	ST7735_WriteString(21, 14+row*12, &Line[row][0], Font_7x10, YELLOW,BLACK);					//marker
}
void points_esc(void)
{
	if(row < 7)
	{
		points_group_ind = row;
		(pp_points_menu[row * MEMORY_SUBPOINTS]->exist_flag)? (current_menu = M_NAVTO_POINTS): (current_menu = M_MAIN);
	}else
	{
		current_device = row - MEMORY_POINT_GROUPS + 1;
		(pp_points_menu[row * MEMORY_SUBPOINTS]->exist_flag)? (current_menu = M_BEACONS): (current_menu = M_MAIN);
	}
}
void draw_show_points(void)
{
	current_point_group = row;
	ST7735_SetRotation(0);
	sprintf(&Line[0][1], " %2d %7s points:", pp_points_menu[current_point_group * MEMORY_SUBPOINTS]->exist_flag, get_points_group_name(current_point_group));
	ST7735_WriteString(0, 0, &Line[0][1], Font_6x8, CYAN,BLACK);

//	row = 0;
	uint8_t k = 0;

	for(uint8_t i = 1; i < MEMORY_SUBPOINTS; i++)
	{
		if(pp_points_menu[current_point_group * MEMORY_SUBPOINTS + i]->exist_flag == 1) {
			sprintf(&Line[k][0], "%1dLatit : %ld", i, pp_points_menu[current_point_group * MEMORY_SUBPOINTS + i]->latitude.as_integer);
		//sprintf(&Line[k][0], "%1dLatit : %ld",i ,pp_points_menu[12]->latitude.as_integer);
			ST7735_WriteString(0, 8+k*11, &Line[k][0], Font_7x10, GREEN,BLACK);
			sprintf(&Line[++k][0], "%1dLongit: %ld", i, pp_points_menu[current_point_group * MEMORY_SUBPOINTS + i]->longitude.as_integer);
		//sprintf(&Line[k][0], "%1dLatit : %ld",i ,pp_points_menu[12]->longitude.as_integer);
			ST7735_WriteString(0, 7+k*11, &Line[k][0], Font_7x10, YLWGRN,BLACK);
			if(++k > 14) i = MEMORY_SUBPOINTS;			//restrict to 14 lines
		}
	}
}
//---------------------------------------------------------------------------
//-----------------------------MEMORY POINTS MENU----------------------------
void draw_device_submenu(void)
{
	for(uint8_t i = 0; i < MEMORY_POINT_GROUPS; i++)							//calculate saved memory points
	{
		pp_points_menu[i * MEMORY_SUBPOINTS]->exist_flag = 0;			//clear sub-point 0
		for(uint8_t j = 1; j < MEMORY_SUBPOINTS; j++)
		{
			if(pp_points_menu[i * MEMORY_SUBPOINTS + j]->exist_flag == 1) {
				pp_points_menu[i * MEMORY_SUBPOINTS]->exist_flag++;	//save to sub-point 0: 0, 28, 55, 83, 111, ...
			}
		}
	}
	ST7735_SetRotation(0);
//	if(((current_device == this_device) && (PVTbuffer[21+6] & 0x01)) || (pp_devices_menu[current_device]->valid_fix_flag))	//if fix valid
	if(pp_devices_menu[current_device]->valid_fix_flag)		//if fix valid for this or current remote device
	{
		sprintf(&Line[0][1], " Save %d as:", current_device);
		ST7735_WriteString(0, -1, &Line[0][1], Font_11x18, CYAN,BLACK);

		row = get_current_item();
		for (uint8_t k = 0; k < MEMORY_POINT_GROUPS; k++)		//draw points groups
		{
			sprintf(&Line[k][0], " %4s   /%1d", get_points_group_short(k), pp_points_menu[k * MEMORY_SUBPOINTS]->exist_flag);	//amount of saved sub-points
			if(k == row) ST7735_WriteString(3, 17+k*18, &Line[k][0], Font_11x18, YELLOW,BLACK);		//active points group
			else ST7735_WriteString(3, 17+k*18, &Line[k][0], Font_11x18, GREEN,BLACK);				//other points groups
		}
		sprintf(&Line[row][0], ">");
		ST7735_WriteString(3, 17+row*18, &Line[row][0], Font_11x18, YELLOW,BLACK);					//marker

	}else {
		sprintf(&Line[3][0], "  NOTHING ");
		sprintf(&Line[5][0], "  TO SAVE");
		ST7735_WriteString(0, 54, &Line[3][0], Font_11x18, CYAN,BLACK);
		ST7735_WriteString(0, 81, &Line[5][0], Font_11x18, CYAN,BLACK);
//		HAL_Delay(300);
		shortBeepsBlocking(3);
		ST7735_FillScreen(BLACK);
		current_menu = M_DEVICES;
	}
}
void draw_set_points(void)
{
		sprintf(&Line[0][1], " Save %d as:", current_device);
		ST7735_WriteString(0, -1, &Line[0][1], Font_11x18, CYAN,BLACK);

	current_point_group = row;
	for (uint8_t k = 0; k < MEMORY_POINT_GROUPS; k++)
	{
		sprintf(&Line[k][0], " %4s  %1d/%1d", get_points_group_short(k), memory_subpoint_ind[k], pp_points_menu[k * MEMORY_SUBPOINTS]->exist_flag);
		if(k == row) {
			ST7735_WriteString(3, 17+k*18, &Line[k][0], Font_11x18, CYAN,BLACK);									//active points group
			if(pp_points_menu[current_point_group * MEMORY_SUBPOINTS + memory_subpoint_ind[k]]->exist_flag == 1) {
				ST7735_WriteString(3, 17+k*18, &Line[k][0], Font_11x18, RED,BLACK);									//if sub-point already exist
			}
		}else ST7735_WriteString(3, 17+k*18, &Line[k][0], Font_11x18, GREEN,BLACK);									//other points groups
	}
	sprintf(&Line[row][0], ">");
	ST7735_WriteString(3, 17+row*18, &Line[row][0], Font_11x18, CYAN,BLACK);										//marker
}
//-------------------------MEMORY POINTS SET----------------------------
void set_subpoint_up(void) {
	if(memory_subpoint_ind[current_point_group] == MEMORY_SUBPOINTS - 1)	//if (memory_subpoint == MEMORY_SUBPOINTS)
    {
		memory_subpoint_ind[current_point_group] = 1;
    }else {
    	memory_subpoint_ind[current_point_group]++;
    }
}
void set_subpoint_down(void) {
    if (memory_subpoint_ind[current_point_group] < 2)
    {
    	memory_subpoint_ind[current_point_group] = MEMORY_SUBPOINTS - 1;
    }else {
    	memory_subpoint_ind[current_point_group]--;
    }
}
void set_subpoint_ok(void) {
    if (memory_subpoint_ind[current_point_group]) {    //if (!= 0) copy current device/point to selected mem point

   	pp_points_menu[current_point_group * MEMORY_SUBPOINTS + memory_subpoint_ind[current_point_group]]->exist_flag = 1;
   	pp_points_menu[current_point_group * MEMORY_SUBPOINTS + memory_subpoint_ind[current_point_group]]->latitude.as_integer = pp_devices_menu[current_device]->latitude.as_integer;
   	pp_points_menu[current_point_group * MEMORY_SUBPOINTS + memory_subpoint_ind[current_point_group]]->longitude.as_integer = pp_devices_menu[current_device]->longitude.as_integer;

   	flag_points_changed = 1;
   	pp_points_menu[current_point_group * MEMORY_SUBPOINTS]->exist_flag = 0;			//clear subpoint 0
   	memory_subpoint_ind[current_point_group] = 0;
    memory_points_save();		//save to flash

    current_menu = M_DEVICE_SUBMENU;
	}else current_menu = M_SET_POINTS;
}

void set_subpoint_esc(void) {
	pp_points_menu[current_point_group * MEMORY_SUBPOINTS]->exist_flag = 0;			//clear subpoint 0
	memory_subpoint_ind[current_point_group] = 0;   //exit no save, reset value
	memory_points_load();
    current_menu = M_DEVICE_SUBMENU;
}
//------------------------------MEMORY POINTS END----------------------------
//---------------------------------------------------------------------------
//-----------------------------SETTINGS MENU---------------------------------
void draw_settings(void)
{
	sprintf(&Line[0][0], "     SETTINGS    ");
	ST7735_WriteString(0, 0, &Line[0][0], Font_7x10, CYAN,BLACK);
//	todo? (p_settings_menu->) instead of (settings_copy_menu.)
	sprintf(&Line[0][0], " Device:%1d/%1d", settings_copy_menu.device_number, settings_copy_menu.devices_on_air);	//this_device);
	sprintf(&Line[1][0], " SprFctr %02d", settings_copy_menu.spreading_factor);		//LORA_SPREADING_FACTOR);

//	sprintf(&Line[2][0], " Region %d", (800 + p_freq_region_values[settings_copy_menu.freq_region_opt]));
	sprintf(&Line[2][0], " CodRate4/%d", p_coding_rate_values[settings_copy_menu.coding_rate_opt]);
	sprintf(&Line[3][0], " Channel %2d", settings_copy_menu.freq_channel);
	sprintf(&Line[4][0], " TxPower%3d", p_tx_power_values[settings_copy_menu.tx_power_opt]);	//settings_copy_menu.tx_power_opt);
	sprintf(&Line[5][0], " TimZone %2d", settings_copy_menu.time_zone_hour);
	sprintf(&Line[6][0], " TimeOut%3d", settings_copy_menu.timeout_threshold);
	sprintf(&Line[7][0], " Fence  %3d", settings_copy_menu.fence_threshold);
	row = get_current_item();
	for (uint8_t k = 0; k < 8; k++)
	{
		if(k == row) ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, YELLOW,BLACK);
		else ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, GREEN,BLACK);
	}
	sprintf(&Line[row][0], ">");
	ST7735_WriteString(3, 10+row*19, &Line[row][0], Font_11x18, YELLOW,BLACK);
}

void draw_set_settings(void)
	{
		sprintf(&Line[0][0], "  MODIFY THE VALUE");
		ST7735_WriteString(0, 0, &Line[0][0], Font_7x10, CYAN,BLACK);

		sprintf(&Line[0][0], " Device:%1d/%1d", settings_copy_menu.device_number, settings_copy_menu.devices_on_air);	//this_device);
		sprintf(&Line[1][0], " SprFctr %02d", settings_copy_menu.spreading_factor);		//LORA_SPREADING_FACTOR);

//		sprintf(&Line[2][0], " Region %d", (800 + p_freq_region_values[settings_copy_menu.freq_region_opt]));
//		sprintf(&Line[3][0], " CodRate4/%d", p_coding_rate_values[settings_copy_menu.coding_rate_opt]);
		sprintf(&Line[2][0], " CR_opt   %d", settings_copy_menu.coding_rate_opt);
		sprintf(&Line[3][0], " Channel %2d", settings_copy_menu.freq_channel);
		sprintf(&Line[4][0], " TxPower%3d", p_tx_power_values[settings_copy_menu.tx_power_opt]);	//settings_copy_menu.tx_power_opt);
		sprintf(&Line[5][0], " TimZone %2d", settings_copy_menu.time_zone_hour);
		sprintf(&Line[6][0], " TimOut %3d", settings_copy_menu.timeout_threshold);
		sprintf(&Line[7][0], " Fence  %3d", settings_copy_menu.fence_threshold);
//		int8_t row = get_current_item();
		for (uint8_t k = 0; k < 8; k++)
		{
			if(k == row) ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, CYAN,BLACK);
			else ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, GREEN,BLACK);
		}
		sprintf(&Line[row][0], ">");
		ST7735_WriteString(3, 10+row*19, &Line[row][0], Font_11x18, CYAN,BLACK);
	}
//---------------------------------SETTINGS SET-----------------------------------
	void set_device_number_up(void)
	{
		if (settings_copy_menu.device_number == settings_copy_menu.devices_on_air)
		{
			settings_copy_menu.device_number = DEVICE_NUMBER_FIRST;

			if (settings_copy_menu.devices_on_air == DEVICE_NUMBER_LAST)
			{
				settings_copy_menu.devices_on_air = DEVICE_NUMBER_FIRST + 2;
			}else {
				settings_copy_menu.devices_on_air++;
			}
		}else {
			settings_copy_menu.device_number++;
		}
	}
	void set_device_number_down(void)
	{
		if (settings_copy_menu.device_number == DEVICE_NUMBER_FIRST)
		{
			if (settings_copy_menu.devices_on_air == (DEVICE_NUMBER_FIRST + 2))
			{
				settings_copy_menu.devices_on_air = DEVICE_NUMBER_LAST;
			}else {
				settings_copy_menu.devices_on_air--;
			}

			settings_copy_menu.device_number = settings_copy_menu.devices_on_air;
		}else {
			settings_copy_menu.device_number--;
		}

	}
	void set_device_number_ok(void)
	{
	    if ((settings_copy_menu.device_number != this_device) | (settings_copy_menu.devices_on_air != p_settings_menu->devices_on_air))
	    {
	        flag_settings_changed = 1;
	    	settings_save(&settings_copy_menu);
	    	settings_load();
	    	pp_devices_menu[this_device]->display_status = 1;		//otherwise it shut off
	    }
	    current_menu = M_SETTINGS;
	}
	void set_device_number_esc(void)
	{
	    settings_copy_menu.device_number = this_device;   			//exit with no save, reset values
	    settings_copy_menu.devices_on_air = p_settings_menu->devices_on_air;
	    current_menu = M_SETTINGS;
	}

	void set_spreading_factor_up(void)
	{
		(settings_copy_menu.spreading_factor > 11)? (settings_copy_menu.spreading_factor = 7): settings_copy_menu.spreading_factor++;
	}
	void set_spreading_factor_down(void)
	{
		(settings_copy_menu.spreading_factor < 8)? (settings_copy_menu.spreading_factor = 12): settings_copy_menu.spreading_factor--;
	}
	void set_spreading_factor_ok(void)
	{
	    if (settings_copy_menu.spreading_factor != p_settings_menu->spreading_factor)
	    {
	    	(settings_copy_menu.spreading_factor == 12)? serialPrint(set_pertriple_period, sizeof(set_pertriple_period)):
	    												 serialPrint(set_persecond_period, sizeof(set_persecond_period));
	        flag_settings_changed = 1;
	    	settings_save(&settings_copy_menu);
	    	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_spreading_factor_esc(void)
	{
	    settings_copy_menu.spreading_factor = p_settings_menu->spreading_factor;   //exit no save, reset values
	    current_menu = M_SETTINGS;
	}
	/********************************REGION****************************/
//	void set_freq_region_up(void) {
//	    if (settings_copy_menu.freq_region_opt == FREQ_REGION_LAST_OPTION)
//	    {
//	        settings_copy_menu.freq_region_opt = FREQ_REGION_FIRST_OPTION;
//	    }else {
//	        settings_copy_menu.freq_region_opt++;
//	    }
//	}
//	void set_freq_region_down(void) {
//	    if (settings_copy_menu.freq_region_opt == FREQ_REGION_FIRST_OPTION)
//	    {
//	        settings_copy_menu.freq_region_opt = FREQ_REGION_LAST_OPTION;
//	    }else {
//	        settings_copy_menu.freq_region_opt--;
//	    }
//	}
//	void set_freq_region_ok(void) {
//		if (settings_copy_menu.freq_region_opt != p_settings_menu->freq_region_opt) {
//			flag_settings_changed = 1;
//			settings_save(&settings_copy_menu);
//	    	settings_load();
//		}
//	    current_menu = M_SETTINGS;
//	}
//	void set_freq_region_esc(void) {
//		settings_copy_menu.freq_region_opt = p_settings_menu->freq_region_opt;   //exit no save, reset value
//	    current_menu = M_SETTINGS;
//	}
	/*********************CODING RATE************************/
	void set_coding_rate_up(void) {
		    if (settings_copy_menu.coding_rate_opt == CODING_RATE_LAST_OPTION)
		    {
		        settings_copy_menu.coding_rate_opt = CODING_RATE_FIRST_OPTION;
		    }else {
		        settings_copy_menu.coding_rate_opt++;
		    }
		}
		void set_coding_rate_down(void) {
		    if (settings_copy_menu.coding_rate_opt == CODING_RATE_FIRST_OPTION)
		    {
		        settings_copy_menu.coding_rate_opt = CODING_RATE_LAST_OPTION;
		    }else {
		        settings_copy_menu.coding_rate_opt--;
		    }
		}
		void set_coding_rate_ok(void) {
		    if (settings_copy_menu.coding_rate_opt != p_settings_menu->coding_rate_opt) {
		    	flag_settings_changed = 1;
		    	settings_save(&settings_copy_menu);
		    	settings_load();
		    }
		    current_menu = M_SETTINGS;
		}
		void set_coding_rate_esc(void) {
		    settings_copy_menu.coding_rate_opt = p_settings_menu->coding_rate_opt;   //exit no save, reset value
		    current_menu = M_SETTINGS;
		}
	/******************************CHANNEL************************/
	void set_freq_channel_up(void) {
	    if (settings_copy_menu.freq_channel >= FREQ_CHANNEL_LAST)
	    {
	        settings_copy_menu.freq_channel = FREQ_CHANNEL_FIRST;
	    }else {
	        settings_copy_menu.freq_channel += 5;
	    }
	}
	void set_freq_channel_down(void) {
	    if (settings_copy_menu.freq_channel <= FREQ_CHANNEL_FIRST)
	    {
	        settings_copy_menu.freq_channel = FREQ_CHANNEL_LAST;
	    }else {
	        settings_copy_menu.freq_channel -= 5;
	    }
	}
	void set_freq_channel_ok(void) {
	    if (settings_copy_menu.freq_channel != p_settings_menu->freq_channel) {
	    	flag_settings_changed = 1;
	    	settings_save(&settings_copy_menu);
	    	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_freq_channel_esc(void) {
	    settings_copy_menu.freq_channel = p_settings_menu->freq_channel;   //exit no save, reset value
	    current_menu = M_SETTINGS;
	}
	/********************************POWER*******************************/
	void set_tx_power_up(void) {
	    if (settings_copy_menu.tx_power_opt == TX_POWER_LAST_OPTION)
	    {
	        settings_copy_menu.tx_power_opt = TX_POWER_FIRST_OPTION;
	    }else {
	        settings_copy_menu.tx_power_opt++;
	    }
	}
	void set_tx_power_down(void) {
	    if (settings_copy_menu.tx_power_opt == TX_POWER_FIRST_OPTION)
	    {
	        settings_copy_menu.tx_power_opt = TX_POWER_LAST_OPTION;
	    }else {
	        settings_copy_menu.tx_power_opt--;
	    }
	}
	void set_tx_power_ok(void) {
	    if (settings_copy_menu.tx_power_opt != p_settings_menu->tx_power_opt) {
	    	flag_settings_changed = 1;
	    	settings_save(&settings_copy_menu);
	    	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_tx_power_esc(void) {
	    settings_copy_menu.tx_power_opt = p_settings_menu->tx_power_opt;   //exit no save, reset value
	    current_menu = M_SETTINGS;
	}
	/*********************************TIMEOUT****************************/
	void set_timeout_up(void) {
		if (settings_copy_menu.timeout_threshold >= TIMEOUT_THRESHOLD_MAX)
		{
			settings_copy_menu.timeout_threshold = TIMEOUT_THRESHOLD_MIN;
		}else {
			settings_copy_menu.timeout_threshold += TIMEOUT_THRESHOLD_STEP;
		}
	}
	void set_timeout_down(void) {
		if (settings_copy_menu.timeout_threshold <= TIMEOUT_THRESHOLD_MIN)
		{
			settings_copy_menu.timeout_threshold = TIMEOUT_THRESHOLD_MAX;
		}else {
			settings_copy_menu.timeout_threshold -= TIMEOUT_THRESHOLD_STEP;
		}
	}
	void set_timeout_ok(void) {
	    if (settings_copy_menu.timeout_threshold != p_settings_menu->timeout_threshold) {
//	    	flag_settings_changed = 1;
	    	settings_save(&settings_copy_menu);
	       	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_timeout_esc(void) {
		settings_copy_menu.timeout_threshold = p_settings_menu->timeout_threshold;   //exit no save, reset value
	    current_menu = M_SETTINGS;
	}
	/*****************************FENCE**********************************/
	void set_fence_up(void) {
		if (settings_copy_menu.fence_threshold >= FENCE_THRESHOLD_MAX)
		{
			settings_copy_menu.fence_threshold = FENCE_THRESHOLD_MIN;
		}else {
			settings_copy_menu.fence_threshold += FENCE_THRESHOLD_STEP;
		}
	}
	void set_fence_down(void) {
		if (settings_copy_menu.fence_threshold <= FENCE_THRESHOLD_MIN)
		{
			settings_copy_menu.fence_threshold = FENCE_THRESHOLD_MAX;
		}else {
			settings_copy_menu.fence_threshold -= FENCE_THRESHOLD_STEP;
		}
	}
	void set_fence_ok(void) {
	    if (settings_copy_menu.fence_threshold != p_settings_menu->fence_threshold) {
//	    	flag_settings_changed = 1;
	    	settings_save(&settings_copy_menu);
	    	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_fence_esc(void) {
		settings_copy_menu.fence_threshold = p_settings_menu->fence_threshold;   //exit no save, reset value
	    current_menu = M_SETTINGS;
	}
	/****************************TIMEZONE*********************************/
	void set_timezone_up(void) {
		(settings_copy_menu.time_zone_hour > 11)? (settings_copy_menu.time_zone_hour = -12): settings_copy_menu.time_zone_hour++;
	}
	void set_timezone_down(void) {
		(settings_copy_menu.time_zone_hour < -11)? (settings_copy_menu.time_zone_hour = 12): settings_copy_menu.time_zone_hour--;
	}
	void set_timezone_ok(void) {
	    if(settings_copy_menu.time_zone_hour != p_settings_menu->time_zone_hour) {
//	        flag_settings_changed = 1;
	        settings_save(&settings_copy_menu);
	       	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_timezone_esc(void) {
		settings_copy_menu.time_zone_hour = p_settings_menu->time_zone_hour;
	    current_menu = M_SETTINGS;
	}
	/**************************CONFIRM SETTINGS*****************************/
void draw_confirm_settings(void)
{
    if (flag_settings_changed)
    {
    	ST7735_SetRotation(0);
    	sprintf(&Line[1][0], "    SETTINGS     ");
    	sprintf(&Line[2][0], "   HAS CHANGED   ");
   		sprintf(&Line[3][0], "            ");
    	for (uint8_t k = 1; k < 4; k++) {
    		ST7735_WriteString(0, k*11, &Line[k][0], Font_7x10, GREEN,BLACK);
    	}

    	sprintf(&Line[4][0], " Press OK  ");
    	ST7735_WriteString(0, 4*11, &Line[4][0], Font_11x18, YELLOW,BLACK);

    	sprintf(&Line[5][0], " REBOOT AND APPLY ");
	    sprintf(&Line[6][0], "            ");
	   	for (uint8_t k = 5; k < 7; k++) {
	    	ST7735_WriteString(0, 11+k*11, &Line[k][0], Font_7x10, GREEN,BLACK);
	   	}

	    sprintf(&Line[7][0], " Press ESC  ");
	    ST7735_WriteString(0, 8*11, &Line[7][0], Font_11x18, YELLOW,BLACK);

	    sprintf(&Line[8][0], " RECHECK SETTINGS ");
//	    sprintf(&Line[8][0], " RESTORE DEFAULTS ");
//	    sprintf(&Line[9][0], "AND RESET COMPASS ");
	    for (uint8_t k = 8; k < 9; k++) {
	    	ST7735_WriteString(0, (k+2)*11, &Line[k][0], Font_7x10, GREEN,BLACK);
	    }
    }else {
        current_menu = M_MAIN;
	//        draw_current_menu();
    }
}
void confirm_settings_reboot(void)
{
	row = 3;
	sprintf(&Line[row][1], "  Saving...");
	ST7735_WriteString(0, row*18, &Line[row][1], Font_11x18, YELLOW,BLACK);

    flag_settings_changed = 0;
   	settings_save(&settings_copy_menu);

    HAL_Delay(1000);
    NVIC_SystemReset();
}
void confirm_settings_restore(void)
{
	row = 3;
	sprintf(&Line[row][1], "Restoring..");

	ST7735_SetRotation(0);
	ST7735_WriteString(0, row*18, &Line[row][1], Font_11x18, YELLOW,BLACK);

	settings_save_default();
//	settings_copy_menu = *p_settings_menu;   //reset to no changes state
    flag_settings_changed = 0;  //clear flag
    HAL_Delay(1000);
    NVIC_SystemReset();
//    current_menu = M_MAIN;
}
//-----------------------------SETTINGS MENU END------------------------
//----------------------------------------------------------------------

void draw_actions(void)
{
//	current_device = this_device;
	sprintf(&Line[0][0], "     ACTIONS    ");
	ST7735_WriteString(0, 0, &Line[0][0], Font_7x10, CYAN,BLACK);

	sprintf(&Line[0][0], " Power OFF");
	(pp_devices_menu[this_device]->emergency_flag)? sprintf(&Line[1][0], " Emerge  on"): sprintf(&Line[1][0], " Emerge off");
	(pp_devices_menu[this_device]->alarm_flag)?     sprintf(&Line[2][0], " Alarm   on"): sprintf(&Line[2][0], " Alarm  ---");
	(pp_devices_menu[this_device]->gather_flag)?    sprintf(&Line[3][0], " Gather  on"): sprintf(&Line[3][0], " Gather ---");
	sprintf(&Line[4][0], " Del points");
	sprintf(&Line[5][0], " SetDefault");
//	(pp_devices_menu[this_device]->beeper_flag)?    sprintf(&Line[6][0], " Beeper  on"): sprintf(&Line[6][0], " Beeper off");
	if(pp_devices_menu[this_device]->beeper_flag == 0) sprintf(&Line[6][0], " Beeper off");
	else if(p_settings_menu->spreading_factor == 12)
    {//SF12 only: invert flags to transmit in the slot, beacon can get: slot1 for beacon2, slot2 for beacon1
		if(pp_devices_menu[this_device]->beeper_flag == 2) sprintf(&Line[6][0], " Beeper1 on");
		else if(pp_devices_menu[this_device]->beeper_flag == 1) sprintf(&Line[6][0], " Beeper2 on");
	}
	else sprintf(&Line[6][0], " Beeper  on");

	(pp_devices_menu[this_device]->flwtrek_flag)?   sprintf(&Line[7][0], " FlwTrek on"): sprintf(&Line[7][0], " FlwTrekOff");

	row = get_current_item();
	for (uint8_t k = 0; k < 8; k++)
	{
		if(k == row) ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, YELLOW,BLACK);
		else {
			ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, GREEN,BLACK);
//			ST7735_WriteString(3, 10+k*19, &Line[k][0], Font_11x18, CYAN,BLACK);
		}
	}
	sprintf(&Line[row][0], ">");
	ST7735_WriteString(3, 10+row*19, &Line[row][0], Font_11x18, YELLOW,BLACK);
}
void power_long(void)
{
	if(current_menu == M_ACTIONS)
	{
		pp_devices_menu[this_device]->emergency_flag = 1;
		set_current_item(1);
	}

	if (current_menu == M_POINTS) {	//otherwise if some point saved, halt after BTN_PWR_LONG immediately
		memory_subpoint_ind[current_point_group] = 0;
		current_point_group = 0;
//		led_blue_off();		//current_point_group has reseted
	}
	return_from_power_menu = current_menu;
	current_menu = M_ACTIONS;

	shortBeeps(2);								//notice power_long and set emergency_flag
//	led_w_on();
//	HAL_Delay(10);
//	led_w_off();
//	draw_current_menu();
}

void actions_ok(void)	//non standard implementation: switch the current item and do the action
{
	switch (get_current_item())
	{
		case M_ACTIONS_I_POWER_OFF:
			led_w_on();
			HAL_Delay(20);
			release_power();
			break;
		case M_ACTIONS_I_EMERGENCY:
			(pp_devices_menu[this_device]->emergency_flag == 0)?
					(pp_devices_menu[this_device]->emergency_flag = 1):
					(pp_devices_menu[this_device]->emergency_flag = 0);
			break;
		case M_ACTIONS_I_ALARM:		//toggle_alarm();
//			(pp_devices_menu[current_device]->alarm_flag == 0)?
//					(pp_devices_menu[current_device]->alarm_flag = 1):
//					(pp_devices_menu[current_device]->alarm_flag = 0);
			break;
		case M_ACTIONS_I_GATHER:	//toggle_gather();
//			(pp_devices_menu[current_device]->gather_flag == 0)?
//					(pp_devices_menu[current_device]->gather_flag = 1):
//					(pp_devices_menu[current_device]->gather_flag = 0);
			break;
		case M_ACTIONS_I_ERASE_POINTS:
			memory_points_erase();
			memory_points_load();
			break;
		case M_ACTIONS_I_SET_DEFAULTS:
			current_menu = M_CONFIRM_RESTORING;
////			settings_save(&settings_copy_menu);
//			row = 3;
//			sprintf(&Line[row][1], "Restoring..");
//			ST7735_SetRotation(0);
//			ST7735_WriteString(0, row*18, &Line[row][1], Font_11x18, YELLOW,BLACK);
//
//			settings_save_default();
//			HAL_Delay(1000);
//			NVIC_SystemReset();
			break;
		case M_ACTIONS_I_BEEPER:	//SF12 only: invert flags to transmit in the slot, beacon can get: slot1 for beacon2, slot2 for beacon1
			if(p_settings_menu->spreading_factor == 12)
			{
				if(pp_devices_menu[this_device]->beeper_flag == 0) pp_devices_menu[this_device]->beeper_flag = 2;		//0x10
				else if(pp_devices_menu[this_device]->beeper_flag == 2) pp_devices_menu[this_device]->beeper_flag = 1;	//0x01
				else pp_devices_menu[this_device]->beeper_flag = 0;														//0x00
			}
			else
			{
				(pp_devices_menu[this_device]->beeper_flag == 0)?
						(pp_devices_menu[this_device]->beeper_flag = 1):
						(pp_devices_menu[this_device]->beeper_flag = 0);
			}
			break;
		case M_ACTIONS_I_FLWTREK:	//initial set in init_gnss() of gnss.c
			(pp_devices_menu[this_device]->flwtrek_flag == 0)?
					(pp_devices_menu[this_device]->flwtrek_flag = 1):
					(pp_devices_menu[this_device]->flwtrek_flag = 0);
			break;
		default:
			break;
	}
}

void actions_esc(void)
{
	if (current_menu == M_ACTIONS) return_from_power_menu = M_MAIN;
//	}
	reset_current_item_in_menu(M_ACTIONS);
	current_menu = return_from_power_menu;
	//draw_current_menu();
}

void draw_restore_defaults(void)
{
    	sprintf(&Line[1][0], "   TO RESTORE    ");
    	sprintf(&Line[2][0], "    DEFAULT      ");
   		sprintf(&Line[3][0], "    SETTINGS     ");
    	for (uint8_t k = 1; k < 4; k++) {
    		ST7735_WriteString(0, k*11, &Line[k][0], Font_7x10, GREEN,BLACK);
    	}

    	sprintf(&Line[4][0], " Press OK  ");
    	ST7735_WriteString(0, 4*11, &Line[4][0], Font_11x18, YELLOW,BLACK);

    	sprintf(&Line[5][0], " APPLY AND REBOOT ");
	    sprintf(&Line[6][0], "            ");
	   	for (uint8_t k = 5; k < 7; k++) {
	    	ST7735_WriteString(0, 11+k*11, &Line[k][0], Font_7x10, GREEN,BLACK);
	   	}

	    sprintf(&Line[7][0], " Press ESC ");
	    ST7735_WriteString(0, 8*11, &Line[7][0], Font_11x18, YELLOW,BLACK);
	    sprintf(&Line[8][0], "CALL MENU ACTIONS ");
	    for (uint8_t k = 8; k < 9; k++) {
	    	ST7735_WriteString(0, (k+2)*11, &Line[k][0], Font_7x10, GREEN,BLACK);
	    }
}
