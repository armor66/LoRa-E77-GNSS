#ifndef _LCD_DISPLAY_H
#define _LCD_DISPLAY_H

#include "main.h"
#include "fonts.h"

#define RGB565(r, g, b)         (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define PI 	3.14159265

//--- готовые цвета ------------------------------
#define   	BLACK   			0x0000
#define   	BLUE    			0x001F
#define   	RED     			0xF800
#define   	GREEN   			0x07E0
#define 	CYAN    			0x07FF
#define 	MAGENTA 			0xF81F
#define 	YELLOW  			0xFFE0
#define 	WHITE   			0xFFFF
#define 	BKGND				0x00A6

#define NAVY        	0x000F
#define DARKGREEN   	0x03E0
#define GREENYELLOW 	0xAFE5
#define	YLWGRN			0xF7E0
#define DARKCYAN    	0x03EF
#define CYANB	 		0x5EFF
#define MAROON      	0x7800
#define PURPLE      	0x780F
#define OLIVE       	0x7BE0
#define LIGHTGREY   	0xC618
#define DARKGREY    	0x7BEF
#define ORANGE      	0xFB80		//0xFD20
#define PINK        	0xF81F
//------------------------------------------------

//-- Битовые маски настройки цветности NV3023 ----
#define ColorMode_65K    	0x50
#define ColorMode_262K   	0x60
#define ColorMode_12bit  	0x03
#define ColorMode_16bit  	0x05
#define ColorMode_18bit  	0x06
#define ColorMode_16M    	0x07
//------------------------------------------------

#define MADCTL_MY  				0x80
#define MADCTL_MX  				0x40
#define MADCTL_MV  				0x20
#define MADCTL_ML  				0x10
#define MADCTL_RGB 				0x00
#define MADCTL_BGR 				0x08
#define MADCTL_MH  				0x04
//-------------------------------------------------


#define SWRESET 						0x01
#define SLPIN   						0x10
#define SLPOUT  						0x11
#define NORON   						0x13
#define INVOFF  						0x20
#define INVON   						0x21
#define DISPOFF 						0x28
#define DISPON  						0x29
#define CASET   						0x2A
#define RASET   						0x2B
#define RAMWR   						0x2C
#define COLMOD  						0x3A
#define MADCTL  						0x36

#ifdef ST7735
extern SPI_HandleTypeDef hspi2;
#define ST7735_SPI_PORT hspi2

#define ST7735_DISPON  0x29
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1
#define ST7735_INVCTR  0xB4
#define ST7735_NORON   0x13
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_DELAY	0x80
#endif

#define LCD_SPI 			SPI2

void st7735_init(uint8_t rotation);
void nv3023_init(void);
void draw_str_by_rows(uint8_t x, uint8_t y, char *str, FontDef_t* Font, uint16_t color, uint16_t bg_color);
void draw_char(uint8_t x, uint8_t y, char ch, FontDef_t* Font, uint16_t color, uint16_t bg_color);
void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void draw_circle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);
void draw_position(uint8_t x0, uint8_t y0, uint8_t r1, double angle, uint8_t r2, uint8_t tag, uint16_t color);
void erase_position(uint8_t x0, uint8_t y0, uint8_t r1, double angle, uint8_t r2);
void fill_rectgl(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void fill_screen(uint16_t color);
void draw_course(uint8_t x0, uint8_t y0, double angle, uint16_t color);
void draw_arrow(uint8_t x0, uint8_t y0, uint8_t r, double angle, uint8_t length, uint16_t color1, uint16_t color2);
void draw_direction(uint8_t x0, uint8_t y0, uint8_t r, double angle, uint16_t color);
void draw_trace(uint8_t x0, uint8_t y0, uint8_t r1, double angle, uint8_t r2, uint16_t color);



#endif // _LCD_DISPLAY_H_
