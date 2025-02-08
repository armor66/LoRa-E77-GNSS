/*
	SPOKE
    
    file: buttons.h
*/



#ifndef BUTTONS_HEADER
#define BUTTONS_HEADER



#define BUTTON_PRESSED_COUNTER_THRESHOLD    (55)       	//short or long click boundary


//Button physical pins 		(pin number)
#define BUTTON_PWR		(1)		//BTN_1
#define BUTTON_UP_OK	(2)		//BTN_2
#define BUTTON_DOWN_ESC	(3)		//BTN_3

//Button actions
#define BTN_NO_ACTION		(0)

#define BTN_PWR				(1)     //PC0
#define BTN_PWR_LONG		(2)

#define BTN_UP				(3)     //PB3
#define BTN_OK				(4)

#define BTN_DOWN			(5)     //PB4
#define BTN_ESC				(6)

void enable_buttons_interrupts(void);
void disable_buttons_interrupts(void);

uint8_t scan_button(uint8_t button);



#endif /*BUTTONS_HEADER*/
