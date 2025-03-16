#include <math.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "gpio.h"
#include "menu.h"
#include "buttons.h"
#include "tim.h"
//#include "timer.h"
//#include "lptim.h"
#include "adc.h"
#include "settings.h"
#include "lrns.h"
#include "gnss.h"
//#include "ST7735.h"
#include "lcd_display.h"

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
//void main_ok(void);
// NAVIGATION MENU
//SUB NAVIGATION MENU
void draw_navigation(void);
//void draw_beacons(void);
void navigation_up(void);
void navigation_down(void);
void navigation_ok(void);

//void beacons_ok(void);
//void beacons_esc(void);

void draw_navto_points(void);
//void navto_points_ok(void);
void navto_points_esc(void);
// DEVICES MENU
void draw_devices(void);
void draw_this_device(void);
void draw_device_submenu(void);
void draw_set_points(void);
//SUB DEVICES MENU
void scroll_devices_up(void);
void scroll_devices_down(void);
void devices_ok(void);
void devices_esc(void);
// POINTS MENU
void draw_points(void);
void points_esc(void);
//void draw_show_points(void);

//SUB POINTS MENU
//void set_subpoint_up(void);
//void set_subpoint_down(void);
//void set_subpoint_ok(void);
//void set_subpoint_esc(void);

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
void draw_confirm_settings(void);
void confirm_settings_reboot(void);
void donot_save_settings(void);
void confirm_settings_restore(void);

//ACTIONS MENU
void draw_actions(void);
void power_long(void);
void actions_ok(void);
void actions_esc(void);
void draw_restore_defaults(void);

//ALL MENUS HERE
enum    //menu number starts from 1, because 0 is used as "end marker" in menu structs
{
    M_MAIN = 1,
    M_NAVIGATION,
	M_BEACONS,

	M_DEVICES,
//	M_DEVICE_SUBMENU,
//	M_SET_POINTS,

	M_NAVTO_POINTS,

	M_POINTS,
//	M_POINTS_SUBMENU,
//	M_POINTS_SUBMENU_DEVICES,

	M_SETTINGS,
	M_CONFIRM_SETTINGS,

	M_SET_DEVICE_NUMBER,
	M_SET_SPREADING_FACTOR,
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
//DEVICE TO SAVE SUBMENU
//enum
//{
//	M_DEVICE_SUBMENU_I_ALARMS = 0,
//	M_DEVICE_SUBMENU_I_GROUP1,
//	M_DEVICE_SUBMENU_I_GROUP2,
//	M_DEVICE_SUBMENU_I_GROUP3,
//	M_DEVICE_SUBMENU_I_GROUP4,
//	M_DEVICE_SUBMENU_I_GROUP5,
//	M_DEVICE_SUBMENU_I_BCNMAN,
//	M_DEVICE_SUBMENU_I_LAST = M_DEVICE_SUBMENU_I_BCNMAN
//};
//POINTS
//enum
//{
//	M_POINTS_I_ALARMS = 0,
//	M_POINTS_I_GROUP1,
//	M_POINTS_I_GROUP2,
//	M_POINTS_I_GROUP3,
//	M_POINTS_I_GROUP4,
//	M_POINTS_I_GROUP5,
//	M_POINTS_I_BCNMAN,
//	M_POINTS_I_SAVED1,
//	M_POINTS_I_SAVED2,
//	M_POINTS_I_SAVED3,
//	M_POINTS_I_SAVED4,
//	M_POINTS_I_SAVED5,
//	M_POINTS_I_LAST = M_POINTS_I_SAVED5
//};
//SETTINGS
enum
{
    M_SETTINGS_I_DEVICE = 0,		// + AMOUNT
	M_SETTINGS_I_SPREADING_FACTOR,	// + MODULATION lora/fsk
	M_SETTINGS_I_CODING_RATE,
	M_SETTINGS_I_FREQ_CHANNEL,
	M_SETTINGS_I_TX_POWER,
	M_SETTINGS_I_TIMEZONE,
	M_SETTINGS_I_TIMEOUT,
	M_SETTINGS_I_FENCE,
    M_SETTINGS_I_LAST = M_SETTINGS_I_FENCE		//copy last item here
};
//ACTIONS SUBMENU
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
//Only exclusive (non default) actions here, for example edit a variable in settings
const struct
{
    uint8_t current_menu;
    uint8_t button_pressed;
    void (*execute_function)(void);
} menu_exclusive_table[] = 
{
//  Current Menu                Button pressed          Action (function name)
//	{M_MAIN,					BTN_OK,					main_ok},
//	{M_MAIN,					BTN_ESC,				set_brightness},

	{M_NAVIGATION,				BTN_UP,					navigation_up},		//change distance range
	{M_NAVIGATION,				BTN_DOWN,				navigation_down},	//change distance range
	{M_NAVIGATION,				BTN_OK,					navigation_ok},
//	{M_NAVIGATION,				BTN_ESC,				navigation_esc},	//to main menu

//	{M_BEACONS,					BTN_UP,					navigation_up},
//	{M_BEACONS,					BTN_DOWN,				navigation_down},
//	{M_BEACONS,					BTN_OK,					beacons_ok},
//	{M_BEACONS,					BTN_ESC,				beacons_esc},

	{M_DEVICES,					BTN_UP,					scroll_devices_up},
	{M_DEVICES,					BTN_DOWN,				scroll_devices_down},
//	{M_DEVICES,					BTN_OK,					devices_ok},
	{M_DEVICES,					BTN_ESC,				devices_esc},

//	{M_SET_POINTS,				BTN_UP,                 set_subpoint_up},
//	{M_SET_POINTS,				BTN_DOWN,               set_subpoint_down},
//	{M_SET_POINTS,				BTN_OK,                 set_subpoint_ok},
//	{M_SET_POINTS,				BTN_ESC,            	set_subpoint_esc},

//	{M_NAVTO_POINTS,			BTN_UP,					navigation_up},
//	{M_NAVTO_POINTS,			BTN_DOWN,				navigation_down},
//	{M_NAVTO_POINTS,			BTN_OK,					navto_points_ok},
	{M_NAVTO_POINTS,			BTN_ESC,				navto_points_esc},

	{M_SET_DEVICE_NUMBER,		BTN_UP,                 set_device_number_up},
	{M_SET_DEVICE_NUMBER,		BTN_DOWN,               set_device_number_down},
	{M_SET_DEVICE_NUMBER,		BTN_OK,                 set_device_number_ok},
	{M_SET_DEVICE_NUMBER,		BTN_ESC,            	set_device_number_esc},

	{M_SET_SPREADING_FACTOR,	BTN_UP,                 set_spreading_factor_up},
	{M_SET_SPREADING_FACTOR,	BTN_DOWN,               set_spreading_factor_down},
	{M_SET_SPREADING_FACTOR,	BTN_OK,                 set_spreading_factor_ok},
	{M_SET_SPREADING_FACTOR,	BTN_ESC,            	set_spreading_factor_esc},

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
	{M_CONFIRM_SETTINGS,		BTN_ESC,				donot_save_settings},

	{M_MAIN,					BTN_PWR_LONG,			power_long},
	{M_NAVIGATION,				BTN_PWR_LONG,			power_long},
	{M_BEACONS,					BTN_PWR_LONG,			power_long},
	{M_DEVICES,					BTN_PWR_LONG,			power_long},
	{M_NAVTO_POINTS,			BTN_PWR_LONG,			power_long},
	{M_POINTS,					BTN_PWR_LONG,			power_long},
	{M_POINTS,					BTN_ESC,				points_esc},
//	{M_POINTS_SUBMENU,			BTN_OK,					draw_clear_group},
//	{M_CONFIRM_CLEARGROUP,		BTN_OK,					confirm_clear_group},
	{M_ACTIONS,					BTN_PWR_LONG,			power_long},
	{M_ACTIONS,					BTN_OK,					actions_ok},
	{M_ACTIONS,					BTN_ESC,				actions_esc},

	{M_CONFIRM_RESTORING,		BTN_OK,					confirm_settings_restore},
    {0, 0, 0}   //end marker
};

//Default behavior (non exclusive) when OK button has been pressed (move forward)
const struct
{
    uint8_t current_menu;
    uint8_t current_item;
    uint8_t next_menu;
} menu_forward_table[] = 
{
//  Current Menu                Current Item                		Next Menu
	{M_MAIN,  				M_MAIN_I_NAVIGATION,		M_NAVIGATION},
	{M_MAIN,    			M_MAIN_I_DEVICES,			M_DEVICES},
	{M_MAIN,    			M_MAIN_I_NAVTO_POINTS,		M_NAVTO_POINTS},
	{M_MAIN,    			M_MAIN_I_POINTS,			M_POINTS},
	{M_MAIN,    			M_MAIN_I_SETTINGS,			M_SETTINGS},
	{M_MAIN,    			M_MAIN_I_ACTIONS,			M_ACTIONS},

	{M_SETTINGS,			M_SETTINGS_I_DEVICE,				M_SET_DEVICE_NUMBER},
	{M_SETTINGS,			M_SETTINGS_I_SPREADING_FACTOR,		M_SET_SPREADING_FACTOR},
	{M_SETTINGS,			M_SETTINGS_I_CODING_RATE,			M_SET_CODING_RATE},
	{M_SETTINGS,			M_SETTINGS_I_FREQ_CHANNEL,			M_SET_FREQ_CHANNEL},
	{M_SETTINGS,			M_SETTINGS_I_TX_POWER,				M_SET_TX_POWER},
	{M_SETTINGS,			M_SETTINGS_I_TIMEZONE,				M_SET_TIMEZONE},
	{M_SETTINGS,			M_SETTINGS_I_TIMEOUT,				M_SET_TIMEOUT},
	{M_SETTINGS,			M_SETTINGS_I_FENCE,					M_SET_FENCE},
    {0, 0, 0}   //end marker
};

//Default behavior (non exclusive) when ESC button has been pressed (move backward)
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
//	{M_DEVICE_SUBMENU,			M_DEVICES},
//	{M_SET_POINTS,				M_DEVICE_SUBMENU},

	{M_NAVTO_POINTS,	        M_MAIN},

	{M_POINTS,	                M_MAIN},
//	{M_POINTS_SUBMENU,			M_POINTS},
//	{M_POINTS_SUBMENU_DEVICES,	M_POINTS},

    {M_SETTINGS,                M_CONFIRM_SETTINGS},	//M_MAIN},
	{M_CONFIRM_SETTINGS,		M_SETTINGS},
    {M_SET_DEVICE_NUMBER,       M_SETTINGS},
	{M_SET_SPREADING_FACTOR,	M_SETTINGS},
	{M_SET_CODING_RATE,			M_SETTINGS},
	{M_SET_FREQ_CHANNEL,		M_SETTINGS},
	{M_SET_TX_POWER,			M_SETTINGS},
	{M_SET_TIMEZONE,	        M_SETTINGS},
	{M_SET_TIMEOUT,				M_SETTINGS},
	{M_SET_FENCE,				M_SETTINGS},

	{M_CONFIRM_RESTORING,		M_ACTIONS},
    {0, 0}      //end marker
};
//Structure with list of menus and real-time values of current item in current menu. Last Item is needed for scroll function
//note: if current menu has no items (like INFO menu) no need to put it in structure below,
//because item functions (get, get last, set) automatically return 0 (which is zero item)
struct
{
    const uint8_t curent_menu;
    uint8_t cur_item;
    const uint8_t last_item;
} item_table[] = 
{
//  Current Menu                Current Item                Last Item in Current Menu
    {M_MAIN,                    M_ALL_I_FIRST,              M_MAIN_I_LAST},
//	{M_DEVICE_SUBMENU,			M_ALL_I_FIRST,				M_DEVICE_SUBMENU_I_LAST},
//	{M_POINTS, 	                M_ALL_I_FIRST,              M_POINTS_I_LAST},
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
//	{M_BEACONS,					draw_beacons},

	{M_DEVICES,                 draw_devices},
//	{M_DEVICE_SUBMENU,     		draw_device_submenu},
//	{M_SET_POINTS,				draw_set_points},

	{M_NAVTO_POINTS,			draw_navto_points},

	{M_POINTS,                  draw_points},
//	{M_POINTS_SUBMENU,			draw_show_points},

    {M_SETTINGS,                draw_settings},
    {M_SET_DEVICE_NUMBER,		draw_set_settings},
	{M_SET_SPREADING_FACTOR,	draw_set_settings},
	{M_SET_CODING_RATE,			draw_set_settings},
	{M_SET_FREQ_CHANNEL,		draw_set_settings},
	{M_SET_TX_POWER,			draw_set_settings},
    {M_SET_TIMEZONE,			draw_set_settings},
	{M_SET_TIMEOUT,				draw_set_settings},
	{M_SET_FENCE,				draw_set_settings},
	{M_CONFIRM_SETTINGS,		draw_confirm_settings},

	{M_ACTIONS,					draw_actions},
	{M_CONFIRM_RESTORING,		draw_restore_defaults},
    {0, 0}      //end mark
};

//#define PVTsize 100
//uint8_t PVTbuffer[PVTsize] = {0};
char string_buffer[20][27];
char *fixType[] = {"NoFix", "DReck", "2DFix", "3DFix", "DReck", "Time"}; 	//only 2 bits used to transmit (first 4 items)

int16_t north = 0;
int16_t north_old = 0;
int16_t azimuth_relative_deg = 0;
double north_rad = 0;
double north_rad_old = 0;
int16_t scaled_dist;
int16_t distance_old[10];
double azimuth_relative_rad;
double azimuth_relative_rad_old[10];

int8_t row;
int8_t this_device;								//device number of this device, see init_menu()
int8_t current_device;
uint8_t current_menu;                               		//Actually Current Menu value (real-time)
uint8_t return_from_power_menu;
//uint8_t return_from_points_menu;
uint8_t flag_settings_changed = 0;							//Have settings been changed?
//uint8_t points_group_ind = 0;
uint8_t *p_coding_rate_values;
uint8_t *p_tx_power_values;

uint16_t range = 30;
uint8_t range_ind = 0;
uint8_t range_scale[] = {1, 2, 4, 8, 16, 32, 64};

struct devices_struct **pp_devices_menu;
struct points_struct **pp_points_menu;

struct settings_struct *p_settings_menu;
struct settings_struct settings_copy_menu;

void init_menu(void)
{
	//Load all devices
	pp_devices_menu = get_devices();
//	pp_points_menu = get_points();
	//Load settings and create a local copy
	p_settings_menu = get_settings();
	settings_copy_menu = *p_settings_menu;
	current_device = this_device = p_settings_menu->device_number;

	p_coding_rate_values = get_coding_rate_values();
	p_tx_power_values = get_tx_power_values();

	fill_screen(BLACK);
    current_menu = M_MAIN;
//    return_from_power_menu = M_MAIN;
    set_current_item(M_MAIN_I_NAVIGATION);

//    lcd_off();		//hence BEACON defined
    main_flags.display_status = 1;
	main_flags.update_screen = 1;
}

//Check for buttons and change menu if needed
void change_menu(uint8_t button_code)
{
//	int8_t point_absolute_index = 0;
	if(main_flags.display_status)	//if lcd is on
	{	//search for exclusive operation for this case
		for (uint8_t i = 0; menu_exclusive_table[i].current_menu; i++)     //until end marker
		{
			if (current_menu == menu_exclusive_table[i].current_menu &&
				button_code == menu_exclusive_table[i].button_pressed)
			{
				fill_screen(BLACK);
				menu_exclusive_table[i].execute_function();
				return;         //exit
			}
		}
//there is no exclusive operations for that case, perform default action
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
				fill_screen(BLACK);
				break;

			case BTN_ESC:
				switch_backward();
				fill_screen(BLACK);
				break;

			case BTN_PWR:
				lcd_off();		//set to 1: lcd_display_off
				main_flags.display_status = 0;
				break;

			default:
				break;
		}	//switch (button_code)
	}else 	//if lcd is off, display_status = 0
	{
		switch (button_code)
		{
			case BTN_PWR_LONG:
				lcd_on();
				main_flags.display_status = 1;
				break;

			case BTN_UP:		//if short pressed
				break;

			case BTN_OK:
				break;			//leave case BTN_OK

			case BTN_DOWN:				//if short pressed
				break;

			case BTN_ESC:
				break;		//leave case BTN_ESC

			default:
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
	if(main_flags.display_status)
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
}
//------------------------------------------------------------------
//---------------------------MAIN MENU------------------------------
void draw_main(void)
{
	uint8_t day = PVTbuffer[7+6];
	uint8_t month = PVTbuffer[6+6];
	uint16_t year = (PVTbuffer[5+6]<<8) + PVTbuffer[4+6];
	uint8_t hour = PVTbuffer[14];
	row = 0;

	if(pp_devices_menu[this_device]->valid_date_flag)		//bit 5: information about UTC Date and Time of Day validity conﬁrmation is available
	{
		year = year - 2000;
		hour = hour + p_settings_menu->time_zone_hour;
		if(hour > 23) hour = hour - 24;
	}else day = month = year = hour = 0;

	if((p_settings_menu->spreading_factor == 12) && (this_device > 2))
	{
		sprintf(&string_buffer[row][0], "!set device 1 or 2");
	}else if(pp_devices_menu[this_device]->batt_voltage < 50)		// U < 3.2volt (!pp_devices_menu[this_device]->batt_voltage)
	{
		sprintf(&string_buffer[row][0], "DevID:%02d  ADC %3d", this_device, main_flags.adc_calibration_factor);
//		sprintf(&string_buffer[row][0], "DevID:%02d Batt low!", this_device);
	}else sprintf(&string_buffer[row][0], "DevID:%02d %d.%02d Volt", this_device, (pp_devices_menu[this_device]->batt_voltage+270)/100,
    															  (pp_devices_menu[this_device]->batt_voltage+270)%100);
	row+=1;	//1
	sprintf(&string_buffer[row][0], "%02d/%02d/%02d  %02d:%02d:%02d", day, month, year, hour, PVTbuffer[15], PVTbuffer[16]);
	for (uint8_t k = 0; k < row+1; k++)
	{
		draw_str_by_rows(0, 1+k*11, &string_buffer[k][0], &Font_7x9, WHITE,BLACK);
	}
	row+=1;	//2
	if(main_flags.nav_pvt_ram_flag)
	{
		sprintf(&string_buffer[row][0], "%d %5s pDop:%2d.%02d", main_flags.fix_valid, fixType[pp_devices_menu[this_device]->fix_type_opt],
						pp_devices_menu[this_device]->p_dop/100, pp_devices_menu[this_device]->p_dop%100);
		(pp_devices_menu[this_device]->valid_fix_flag)?
				draw_str_by_rows(0, 1+row*11, &string_buffer[row][0], &Font_7x9, WHITE,BLACK):
//				draw_str_by_rows(0, 1+row*11, &string_buffer[row][0], &Font_7x9, CYAN,BLACK);
				((main_flags.GPScold_restarted)?
						draw_str_by_rows(0, 1+row*11, &string_buffer[row][0], &Font_7x9, ORANGE,BLACK):
						draw_str_by_rows(0, 1+row*11, &string_buffer[row][0], &Font_7x9, CYAN,BLACK));
	}
	else draw_str_by_rows(0, 1+row*11, "GPS not configured", &Font_7x9, ORANGE,BLACK);

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
		sprintf(&string_buffer[row][0], "Dev%d set EMERGENCY", dev);
		draw_str_by_rows(0, 1+row*11, &string_buffer[row][0], &Font_7x9, CYAN,BLACK);
	}
	else
	{
		int8_t nods_on_the_air = (pp_devices_menu[1]->valid_fix_flag) + (pp_devices_menu[2]->valid_fix_flag) + (pp_devices_menu[3]->valid_fix_flag) +
																		(pp_devices_menu[4]->valid_fix_flag) + (pp_devices_menu[5]->valid_fix_flag);
		sprintf(&string_buffer[row][0], "OnTheAir: %d/%d Devs", nods_on_the_air, p_settings_menu->devices_on_air);
		draw_str_by_rows(0, 1+row*11, &string_buffer[row][0], &Font_7x9, WHITE,BLACK);
	}

	sprintf(&string_buffer[2][0], " Navigation");
	sprintf(&string_buffer[3][0], " Devices   ");
	sprintf(&string_buffer[4][0], " Nav2points");
	sprintf(&string_buffer[5][0], " Points    ");
	sprintf(&string_buffer[6][0], " Settings  ");
	sprintf(&string_buffer[7][0], " Actions   ");
	row = 2 + get_current_item();
	for (uint8_t k = 2; k < 8; k++)
	{
		if(k == row) draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, YELLOW,BLACK);
		else draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, GREEN,BLACK);
	}
//	draw_char(3, 10+row*19, *">", &Font_11x18, YELLOW,BLACK);	//">"
	int8_t slot = main_flags.time_slot + 1;
	if(main_flags.time_slot == p_settings_menu->devices_on_air) slot = 1;
	if((p_settings_menu->spreading_factor == 12) && (main_flags.time_slot == 2)) slot = 1;
	sprintf(&string_buffer[row][0], "%d",slot);
	draw_str_by_rows(3, 10+row*19,  &string_buffer[row][0], &Font_11x18, YELLOW,BLACK);
}
//--------------------------------MAIN MENU END------------------------------
//----------------------------------------------------------------------------
//--------------------------------NAVIGATION MENU--------------------------------
int16_t heading_deg = 0;
double heading_rad = 0;
void draw_navigation(void)	//int8_t menu)
{
	row = 0;
	uint8_t dev = 0;

	if(pp_devices_menu[this_device]->gps_speed > GPS_SPEED_THRS)
	{
		sprintf(&string_buffer[row][4], "%2dkmh", pp_devices_menu[this_device]->gps_speed);
		draw_str_by_rows(37, row*19, &string_buffer[row][4], &Font_11x18, GREEN,BLACK);
	 	(heading_deg < 100)? (sprintf(&string_buffer[row+=1][4], "%3d%% ", heading_deg)): (sprintf(&string_buffer[row+=1][4], " %3d%%", heading_deg));	//just shift->
	 	draw_str_by_rows(37, (row)*19, &string_buffer[row][4], &Font_11x18, YELLOW,BLACK);
	}else
	{
//		sprintf(&string_buffer[row][4], );
		draw_str_by_rows(37, row*19, "Steer", &Font_11x18, ORANGE,BLACK);
//		sprintf(&string_buffer[row+=1][4], );
		draw_str_by_rows(37, (row+=1)*19, "North", &Font_11x18, CYANB,BLACK);
	}

	draw_arrow(63, 97, 57, north_rad, 28, CYANB, RED);

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

	pp_devices_menu[dev]->beacon_flag? sprintf(&string_buffer[0][0], "Bcn:%d", dev): sprintf(&string_buffer[0][0], "Dev:%d", dev);	//if is beacon

		azimuth_relative_deg = azimuth_deg_signed[dev] - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
		sprintf(&string_buffer[1][0], "%4d%%", azimuth_relative_deg);
		sprintf(&string_buffer[2][0], "%4dm", ((uint16_t)distance[dev] & 0x1FFF));
		sprintf(&string_buffer[3][0], "%4ddB", pp_devices_menu[dev]->rssi);		//(int8_t)buffer[BUFFER_AIR_SIZE]);

		for (uint8_t k = 0; k < 4; k++) {
			if(pp_devices_menu[dev]->valid_fix_flag) draw_str_by_rows(0, k*11, &string_buffer[k][0], &Font_7x9, YELLOW,BLACK);		//if remote fix valid (validFixFlag[dev])
			else draw_str_by_rows(0, k*11, &string_buffer[k][0], &Font_7x9, MAGENTA,BLACK);
		}

	for(dev = p_settings_menu->devices_on_air; dev > 0; dev--)
	{
		if(dev != this_device)
		{
			if(pp_devices_menu[dev]->valid_fix_flag) break;
		}
	}

	pp_devices_menu[dev]->beacon_flag? sprintf(&string_buffer[0][13], "Bcn:%d", dev): sprintf(&string_buffer[0][13], "Dev:%d", dev);	//if is beacon

		azimuth_relative_deg = azimuth_deg_signed[dev] - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;

		sprintf(&string_buffer[1][13], "%4d%%", azimuth_relative_deg);
		sprintf(&string_buffer[2][13], "%4dm", ((uint16_t)distance[dev] & 0x1FFF));
		sprintf(&string_buffer[3][13], "%4dd", pp_devices_menu[dev]->rssi);		//(int8_t)buffer[BUFFER_AIR_SIZE]);

		for (uint8_t k = 0; k < 4; k++) {
			if(pp_devices_menu[dev]->valid_fix_flag) draw_str_by_rows(91, k*11, &string_buffer[k][13], &Font_7x9, YELLOW,BLACK);		//if remote fix valid (validFixFlag[dev])
			else draw_str_by_rows(91, k*11, &string_buffer[k][13], &Font_7x9, MAGENTA,BLACK);
		}

	for(uint8_t i = 0; i < (p_settings_menu->devices_on_air + 1); i++) {						//devises on the air = 3
		if(i == this_device) {										//if this device
			draw_position(63, 97, 0, 0, 7, i, GREEN);
		}else if (pp_devices_menu[i]->valid_fix_flag) {									//if remote fix valid
			scaled_dist = ((int16_t)distance[i] & 0x1FFF)*2  / range_scale[range_ind];
			azimuth_relative_rad = azimuth_rad[i] - heading_rad;
			erase_position(63, 97, distance_old[i], azimuth_relative_rad_old[i], 8);
			if(scaled_dist > range * 2)
			{
				scaled_dist = range * 2;
				draw_position(63, 97, scaled_dist, azimuth_relative_rad, 7, i, RED);
			}else draw_position(63, 97, scaled_dist, azimuth_relative_rad, 7, i, YELLOW);
			distance_old[i] = scaled_dist;
			azimuth_relative_rad_old[i] = azimuth_relative_rad;
		}else if(distance_old[i] > 3) erase_position(63, 97, distance_old[i], azimuth_relative_rad_old[i], 8);	//erase or change CYAN to MAGENTA
	}		//if(distance_old[i] > 3) -> don't erase this device (yellow number)

	draw_circle(63, 97, 60, WHITE);
	row = 13;
	sprintf(&string_buffer[row][0], "%d.%02dV", (pp_devices_menu[this_device]->batt_voltage+270)/100,
		  	   	   	   	   	    		 (pp_devices_menu[this_device]->batt_voltage+270)%100);
	draw_str_by_rows(0, 7+row*11, &string_buffer[row][0], &Font_7x9, GREEN,BLACK);
	sprintf(&string_buffer[row][13], "%4dm", range * range_scale[range_ind]);
	draw_str_by_rows(92, 7+row*11, &string_buffer[row][13], &Font_7x9, WHITE,BLACK);
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
void navigation_ok(void) {
	current_menu = M_NAVIGATION;
}
//--------------------------------NAVIGATION MENU END--------------------------------
//-----------------------------------------------------------------------------------
//--------------------------------DEVICES MENU----------------------------
void draw_this_device(void)
{
	uint8_t day = PVTbuffer[7+6];
	uint8_t month = PVTbuffer[6+6];
	uint16_t year = (PVTbuffer[5+6]<<8) + PVTbuffer[4+6];
	uint8_t hour = PVTbuffer[14];
	row = 0;

	if(pp_devices_menu[this_device]->valid_date_flag)	//bit 5: information about UTC Date and Time of Day validity conﬁrmation is available
	{
		year = year - 2000;
		hour = hour + p_settings_menu->time_zone_hour;
		if(hour > 23) hour = hour - 24;
	}else day = month = year = hour = 0;

		sprintf(&string_buffer[0][0], " Device: %d ", this_device);
			if(pp_devices_menu[this_device]->valid_fix_flag) draw_str_by_rows(0, 0, &string_buffer[0][0], &Font_11x18, YELLOW,BLACK);		//if fix valid
			else draw_str_by_rows(0, 0, &string_buffer[0][0], &Font_11x18, MAGENTA,BLACK);

		sprintf(&string_buffer[2][0], "%2d %3d.%03dMHz SF%02d", p_settings_menu->freq_channel,
		(433000 + 50 + p_settings_menu->freq_channel * 25)/1000, (433000 + 50 + p_settings_menu->freq_channel * 25)%1000,
													      p_settings_menu->spreading_factor);
		sprintf(&string_buffer[3][0], "TxPWR %2ddBm CR=4/%d", p_tx_power_values[p_settings_menu->tx_power_opt], p_coding_rate_values[p_settings_menu->coding_rate_opt]);
		sprintf(&string_buffer[4][0], "CPU: %02d%%C %d.%02dVolt",pp_devices_menu[this_device]->core_temperature,
													 (pp_devices_menu[this_device]->core_voltage+270)/100,
				  	  	  	  	  	  	  	  	  	 (pp_devices_menu[this_device]->core_voltage+270)%100);
		sprintf(&string_buffer[5][0], "Battery:  %d.%02dVolt", (pp_devices_menu[this_device]->batt_voltage+270)/100,
 	  	  	  	  	  	  	  	   	   	   	   	    (pp_devices_menu[this_device]->batt_voltage+270)%100);
		sprintf(&string_buffer[6][0], "0x%lX Flag%02X%02d",
						main_flags.settings_address, p_settings_menu->settings_init_flag, main_flags.settings_index);
		for (uint8_t k = 2; k < 7; k++)
		{
			//ST7735_WriteString(0, k*9, string_buffer[k], Font_6x8, GREEN,BLACK);
			draw_str_by_rows(0, k*11, &string_buffer[k][0], &Font_7x9, YELLOW,BLACK);
		}

		sprintf(&string_buffer[7][0], " %5s pDop:%2d.%02d ", fixType[pp_devices_menu[current_device]->fix_type_opt],
				pp_devices_menu[current_device]->p_dop/100, pp_devices_menu[current_device]->p_dop%100);

		sprintf(&string_buffer[8][0], "%02d/%02d/%02d  %02d:%02d:%02d", day, month, year, hour, PVTbuffer[15], PVTbuffer[16]);

		sprintf(&string_buffer[9][0], "Latit : %ld", pp_devices_menu[this_device]->latitude.as_integer);	//((int32_t)(PVTbuffer[37]<<24)+(PVTbuffer[36]<<16)+(PVTbuffer[35]<<8)+PVTbuffer[34]));
		sprintf(&string_buffer[10][0], "Longit: %ld", pp_devices_menu[this_device]->longitude.as_integer);	//((int32_t)(PVTbuffer[33]<<24)+(PVTbuffer[32]<<16)+(PVTbuffer[31]<<8)+PVTbuffer[30]));
		sprintf(&string_buffer[11][0], "HeightMSL:  %4dm", ((PVTbuffer[39+6]<<24)+(PVTbuffer[38+6]<<16)+(PVTbuffer[37+6]<<8)+PVTbuffer[36+6])/1000);
		sprintf(&string_buffer[12][0], "GNSSspeed:%3dkm/h", pp_devices_menu[this_device]->gps_speed);

		if(pp_devices_menu[this_device]->gps_speed > GPS_SPEED_THRS) {
				sprintf(&string_buffer[13][0], "HeadingGNSS:%04d%% ", pp_devices_menu[this_device]->gps_heading);
		}else 	sprintf(&string_buffer[13][0], " No Magn Compass");

		for (uint8_t k = 7; k < 14; k++)
		{
			if(pp_devices_menu[current_device]->valid_fix_flag) draw_str_by_rows(0, 4+k*11, &string_buffer[k][0], &Font_7x9, GREEN,BLACK);	//(PVTbuffer[21+6] & 0x01)
			else draw_str_by_rows(0, 4+k*11, &string_buffer[k][0], &Font_7x9, MAGENTA,BLACK);
		}
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
				sprintf(&string_buffer[0][0], "tBcn%d data:", pp_devices_menu[current_device]->device_num);			//(buffer[0] & 0x03));
			}else sprintf(&string_buffer[0][0], " Dev%d data:", pp_devices_menu[current_device]->device_num);		//(buffer[0] & 0x03));
		}
		else sprintf(&string_buffer[0][0], " NoDevice %d", current_device);

		draw_str_by_rows(0, 0, &string_buffer[0][0], &Font_11x18, YELLOW,BLACK);

		//fixType only 2 bits used to transmit, pDop/10 (0...25.5)	buffer[15]/10, buffer[15]%10);
		sprintf(&string_buffer[2][0], " %5s pDop:%2d.%d", fixType[pp_devices_menu[current_device]->fix_type_opt],
				pp_devices_menu[current_device]->p_dop/10, pp_devices_menu[current_device]->p_dop%10);

//	sprintf(&string_buffer[3][0], "Date../09 %02d:%02d:%02d", pp_devices_menu[current_device]->time_hours,
//			pp_devices_menu[current_device]->time_minutes, pp_devices_menu[current_device]->time_seconds);
		sprintf(&string_buffer[3][0], "Distance:%5d m", ((uint16_t)distance[current_device] & 0xFFFF));
		azimuth_relative_deg = azimuth_deg_signed[current_device] - heading_deg;
		if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
		if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
		sprintf(&string_buffer[4][0], "AzRelative: %03d%%", azimuth_relative_deg);

		sprintf(&string_buffer[5][0], "Speed:   %3d km/h", pp_devices_menu[current_device]->gps_speed);
		if(pp_devices_menu[current_device]->gps_speed)
		{
			sprintf(&string_buffer[6][0], "HeadingGPS:  %03d%% ", pp_devices_menu[current_device]->gps_heading);
		}else 	sprintf(&string_buffer[6][0], "Device%d not moving", current_device);

//	sprintf(&string_buffer[6][0], "Azimuth_s : %03d%%", azimuth_deg_signed[current_device]);
//	sprintf(&string_buffer[7][0], "Azimuth_u : %03d%%", azimuth_deg_unsigned[current_device]);
		sprintf(&string_buffer[7][0], "                  ");

		if(pp_devices_menu[current_device]->batt_voltage < 32)
		{
			sprintf(&string_buffer[8][0], "Battery :  low    ");		//<=3.2 volt
		} else sprintf(&string_buffer[8][0], "Battery: %d.%d Volt ", pp_devices_menu[current_device]->batt_voltage/10, pp_devices_menu[current_device]->batt_voltage%10);

		sprintf(&string_buffer[9][0], "RSSI: %ddBm", pp_devices_menu[current_device]->rssi);
		sprintf(&string_buffer[10][0], " SNR: %02ddB", pp_devices_menu[current_device]->snr);

		(p_settings_menu->spreading_factor == 12)? sprintf(&string_buffer[11][0], "                  "):
		sprintf(&string_buffer[11][0], "RX%d to TX%d: %4dmS", pp_devices_menu[current_device]->device_num, this_device, main_flags.endRX_2_TX);

		sprintf(&string_buffer[12][0], "Latit : %ld", pp_devices_menu[current_device]->latitude.as_integer);
		sprintf(&string_buffer[13][0], "Longit: %ld", pp_devices_menu[current_device]->longitude.as_integer);

		for (uint8_t k = 2; k < 14; k++)
		{
			if(pp_devices_menu[current_device]->valid_fix_flag) draw_str_by_rows(0, 4+k*11, &string_buffer[k][0], &Font_7x9, GREEN,BLACK);	//if remote fix valid (validFixFlag[current_device]),((buffer[14] & 0x10) >> 4)
			else draw_str_by_rows(0, 4+k*11, &string_buffer[k][0], &Font_7x9, MAGENTA,BLACK);
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
//	current_menu = M_DEVICE_SUBMENU;
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
	draw_str_by_rows(0, 70, "  Nothing ", &Font_11x18, CYAN,BLACK);
	draw_str_by_rows(6, 90, "   Here   ", &Font_11x18, CYAN,BLACK);
}
//------------------------------NAV TO POINTS MENU SET----------------------------
void navto_points_ok(void)
{
	current_menu = M_MAIN;
}
void navto_points_esc(void)
{
	current_menu = M_MAIN;			//M_NAVIGATION;
}
//------------------------------NAV TO POINTS MENU END----------------------------
//----------------------------------------------------------------------
//----------------------------------POINTS MENU--------------------------------
void draw_points(void)
{
	draw_str_by_rows(0, 70, "  Nothing ", &Font_11x18, CYAN,BLACK);
	draw_str_by_rows(6, 90, "   Here   ", &Font_11x18, CYAN,BLACK);
}
void points_esc(void)
{
	current_menu = M_MAIN;
}
//------------------------------POINTS END----------------------------
//---------------------------------------------------------------------------
//-----------------------------SETTINGS MENU---------------------------------
void draw_settings(void)
{
	//	sprintf(&string_buffer[0][0], "     SETTINGS    ");
		draw_str_by_rows(0, 0, "     SETTINGS     ", &Font_7x9, CYAN,BLACK);
	//	todo? (p_settings_menu->) instead of (settings_copy_menu.)
		sprintf(&string_buffer[0][0], " Device:%1d/%1d", settings_copy_menu.device_number, settings_copy_menu.devices_on_air);	//this_device);
		sprintf(&string_buffer[1][0], " SprFctr %02d", settings_copy_menu.spreading_factor);		//LORA_SPREADING_FACTOR);

	//	sprintf(&string_buffer[2][0], " Region %d", (800 + p_freq_region_values[settings_copy_menu.freq_region_opt]));
		sprintf(&string_buffer[2][0], " CodRate4/%d", p_coding_rate_values[settings_copy_menu.coding_rate_opt]);
		sprintf(&string_buffer[3][0], " Channel %2d", settings_copy_menu.freq_channel);
		sprintf(&string_buffer[4][0], " TxPower%3d", p_tx_power_values[settings_copy_menu.tx_power_opt]);	//settings_copy_menu.tx_power_opt);
		sprintf(&string_buffer[5][0], " TimZone %2d", settings_copy_menu.time_zone_hour);
		sprintf(&string_buffer[6][0], " TimeOut%3d", settings_copy_menu.timeout_threshold);
		sprintf(&string_buffer[7][0], " Fence  %3d", settings_copy_menu.fence_threshold);
		row = get_current_item();
		for (uint8_t k = 0; k < 8; k++)
		{
			if(k == row) draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, YELLOW,BLACK);
			else draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, GREEN,BLACK);
		}
	//	sprintf(&string_buffer[row][0], ">");
		draw_char(3, 10+row*19, 62, &Font_11x18, YELLOW,BLACK);
}

void draw_set_settings(void)
	{
	//		sprintf(&string_buffer[0][0], "  MODIFY THE VALUE");
			draw_str_by_rows(0, 0, "  MODIFY THE VALUE", &Font_7x9, CYAN,BLACK);

			sprintf(&string_buffer[0][0], " Device:%1d/%1d", settings_copy_menu.device_number, settings_copy_menu.devices_on_air);	//this_device);
			sprintf(&string_buffer[1][0], " SprFctr %02d", settings_copy_menu.spreading_factor);		//LORA_SPREADING_FACTOR);

			sprintf(&string_buffer[2][0], " CR_opt   %d", settings_copy_menu.coding_rate_opt);
			sprintf(&string_buffer[3][0], " Channel %2d", settings_copy_menu.freq_channel);
			sprintf(&string_buffer[4][0], " TxPower%3d", p_tx_power_values[settings_copy_menu.tx_power_opt]);	//settings_copy_menu.tx_power_opt);
			sprintf(&string_buffer[5][0], " TimZone %2d", settings_copy_menu.time_zone_hour);
			sprintf(&string_buffer[6][0], " TimOut %3d", settings_copy_menu.timeout_threshold);
			sprintf(&string_buffer[7][0], " Fence  %3d", settings_copy_menu.fence_threshold);
	//		int8_t row = get_current_item();
			for (uint8_t k = 0; k < 8; k++)
			{
				if(k == row) draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, CYAN,BLACK);
				else draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, GREEN,BLACK);
			}
			sprintf(&string_buffer[row][0], ">");
			draw_char(3, 10+row*19, 62, &Font_11x18, CYAN,BLACK);
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
//	    	settings_save(&settings_copy_menu);
//	    	settings_load();
	    	main_flags.display_status = 1;		//otherwise it shut off
	    }
	    current_menu = M_SETTINGS;
	}
	void set_device_number_esc(void)
	{
	    settings_copy_menu.device_number = this_device;   			//exit with no save, reset values
	    settings_copy_menu.devices_on_air = p_settings_menu->devices_on_air;
	    current_menu = M_SETTINGS;
	}
	/*********************SPREADING FACTOR************************/
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
//	    	(settings_copy_menu.spreading_factor == 12)? serialPrint(set_pertriple_period, sizeof(set_pertriple_period)):
//	    												 serialPrint(set_persecond_period, sizeof(set_persecond_period));
	        flag_settings_changed = 1;
//	    	settings_save(&settings_copy_menu);
//	    	settings_load();
	    }
	    current_menu = M_SETTINGS;
	}
	void set_spreading_factor_esc(void)
	{
	    settings_copy_menu.spreading_factor = p_settings_menu->spreading_factor;   //exit no save, reset values
	    current_menu = M_SETTINGS;
	}
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
//		    	settings_save(&settings_copy_menu);
//		    	settings_load();
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
//	    	settings_save(&settings_copy_menu);
//	    	settings_load();
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
//	    	settings_save(&settings_copy_menu);
//	    	settings_load();
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
    	sprintf(&string_buffer[1][0], "    SETTINGS     ");
    	sprintf(&string_buffer[2][0], "   HAS CHANGED   ");
   		sprintf(&string_buffer[3][0], "            ");
    	for (uint8_t k = 1; k < 4; k++) {
    		draw_str_by_rows(0, k*11, &string_buffer[k][0], &Font_7x9, GREEN,BLACK);
    	}

    	draw_str_by_rows(0, 4*11, " Press OK  ", &Font_11x18, YELLOW,BLACK);

   		draw_str_by_rows(0, 6*11, " REBOOT AND APPLY", &Font_7x9, GREEN,BLACK);

	   	draw_str_by_rows(0, 8*11, " Press ESC ", &Font_11x18, YELLOW,BLACK);

	    sprintf(&string_buffer[8][0], "  TO NOT SAVE ");
	    sprintf(&string_buffer[9][0], "   AND REBOOT ");
	    for (uint8_t k = 8; k < 10; k++) {
	    	draw_str_by_rows(0, (k+2)*11, &string_buffer[k][0], &Font_7x9, GREEN,BLACK);
	    }
    }else current_menu = M_MAIN;
}
void confirm_settings_reboot(void)
{
	fill_screen(BLACK);
	row = 4;
	draw_str_by_rows(0, row*18, " Saving...", &Font_11x18, YELLOW,BLACK);

    flag_settings_changed = 0;
   	settings_save(&settings_copy_menu);

    HAL_Delay(1000);
    NVIC_SystemReset();
}

void donot_save_settings(void)
{
	fill_screen(BLACK);

	row = 4;
	draw_str_by_rows(0, row*18, "Restoring..", &Font_11x18, YELLOW,BLACK);
//	settings_copy_menu = *p_settings_menu;   //reset to no changes state
	HAL_Delay(1000);
	NVIC_SystemReset();
}
//-----------------------------SETTINGS MENU END------------------------
//----------------------------------------------------------------------
//----------------------------- MENU ACTIONS------------------------
void draw_actions(void)
{
//	current_device = this_device;
	draw_str_by_rows(0, 0, "     ACTIONS    ", &Font_7x9, CYAN,BLACK);

	sprintf(&string_buffer[0][0], " Power OFF");
	(pp_devices_menu[this_device]->emergency_flag)? sprintf(&string_buffer[1][0], " Emerge  on"): sprintf(&string_buffer[1][0], " Emerge off");
	(pp_devices_menu[this_device]->alarm_flag)?     sprintf(&string_buffer[2][0], " Alarm   on"): sprintf(&string_buffer[2][0], " Alarm  ---");
	(pp_devices_menu[this_device]->gather_flag)?    sprintf(&string_buffer[3][0], " Gather  on"): sprintf(&string_buffer[3][0], " Gather ---");
	sprintf(&string_buffer[4][0], "           ");
	sprintf(&string_buffer[5][0], " SetDefault");
//	(pp_devices_menu[this_device]->beeper_flag)?    sprintf(&string_buffer[6][0], " Beeper  on"): sprintf(&string_buffer[6][0], " Beeper off");
	if(pp_devices_menu[this_device]->beeper_flag == 0) sprintf(&string_buffer[6][0], "           ");
	else if(p_settings_menu->spreading_factor == 12)
    {//SF12 only: invert flags to transmit in the slot, beacon can get: slot1 for beacon2, slot2 for beacon1
		if(pp_devices_menu[this_device]->beeper_flag == 2) sprintf(&string_buffer[6][0], "           ");
		else if(pp_devices_menu[this_device]->beeper_flag == 1) sprintf(&string_buffer[6][0], "           ");
	}
	else sprintf(&string_buffer[6][0], "           ");

	(pp_devices_menu[this_device]->flwtrek_flag)?   sprintf(&string_buffer[7][0], "           "): sprintf(&string_buffer[7][0], "           ");

	row = get_current_item();
	for (uint8_t k = 0; k < 8; k++)
	{
		if(k == row) draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, YELLOW,BLACK);
		else {
			draw_str_by_rows(3, 10+k*19, &string_buffer[k][0], &Font_11x18, GREEN,BLACK);
//			ST7735_WriteString(3, 10+k*19, &string_buffer[k][0], &Font_11x18, CYAN,BLACK);
		}
	}
//	sprintf(&string_buffer[row][0], ">");
	draw_char(3, 10+row*19, 62, &Font_11x18, YELLOW,BLACK);
}

void power_long(void)
{
	return_from_power_menu = current_menu;
	current_menu = M_ACTIONS;

	shortBeeps(2);			//notice power_long and set emergency_flag
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
			break;
		case M_ACTIONS_I_ALARM:		//toggle_alarm();
			break;
		case M_ACTIONS_I_GATHER:	//toggle_gather();
			break;
		case M_ACTIONS_I_ERASE_POINTS:
			break;
		case M_ACTIONS_I_SET_DEFAULTS:
			current_menu = M_CONFIRM_RESTORING;
			break;
		case M_ACTIONS_I_BEEPER:	//toggle_trigger();
			break;
		case M_ACTIONS_I_FLWTREK:	//initial set in init_gnss() of gnss.c
			break;
		default:
			break;
	}
}

void actions_esc(void)
{
	if (current_menu == M_ACTIONS) return_from_power_menu = M_MAIN;
	reset_current_item_in_menu(M_ACTIONS);
	current_menu = return_from_power_menu;
}

void draw_restore_defaults(void)
{
    	sprintf(&string_buffer[1][0], "   TO RESTORE    ");
    	sprintf(&string_buffer[2][0], "    DEFAULT      ");
   		sprintf(&string_buffer[3][0], "    SETTINGS     ");
    	for (uint8_t k = 1; k < 4; k++) {
    		draw_str_by_rows(0, k*11, &string_buffer[k][0], &Font_7x9, CYAN,BLACK);
    	}

    	draw_str_by_rows(0, 5*11, "Press OK to", &Font_11x18, YELLOW,BLACK);

    	draw_str_by_rows(0, 7*11, "RESTORE AND REBOOT", &Font_7x9, GREEN,BLACK);
    	draw_str_by_rows(0, 8*11, "CALIBRATE COMPASS!", &Font_7x9, RED,BLACK);

    	draw_str_by_rows(0, 10*11, " Press ESC ", &Font_11x18, YELLOW,BLACK);
    	draw_str_by_rows(0, 12*11, "CALL MENU ACTIONS ", &Font_7x9, GREEN,BLACK);
}

void confirm_settings_restore(void)
{
	fill_screen(BLACK);

	row = 4;
	draw_str_by_rows(0, row*18, "Restoring..", &Font_11x18, YELLOW,BLACK);

	settings_save_default(p_settings_menu);

    HAL_Delay(1000);
    NVIC_SystemReset();
}
