/*
    SPOKE
    
    file: menu.h
*/



#ifndef MENU_HEADER
#define MENU_HEADER

#include "main.h"

void init_menu(void);
void change_menu(uint8_t button_code);
void draw_current_menu(void);
void draw_menu(int8_t menu);
void to_halt(void);

#endif /*MENU_HEADER*/
