#ifndef MENU_HEADER
#define MENU_HEADER

#include "main.h"

void init_menu(void);
void change_menu(uint8_t button_code);
void draw_current_menu(void);
void show_settings(void);
void draw_menu(int8_t menu);
void set_brightness(void);

#endif /*MENU_HEADER*/
