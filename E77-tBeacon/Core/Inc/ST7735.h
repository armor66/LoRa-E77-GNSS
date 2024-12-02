#ifndef __ST7735_H__
#define __ST7735_H__

#include "main.h"
#include "fonts.h"
#include "stm32wlxx_hal.h"
#include <stdbool.h>


extern SPI_HandleTypeDef hspi2;
#define ST7735_SPI_PORT hspi2

/****** PIN DEFINES ******/
#define CS_PORT CS_GPIO_Port	//GPIOB
#define CS_PIN  CS_Pin			//GPIO_PIN_10
#define DC_PORT DC_GPIO_Port	//GPIOB
#define DC_PIN  DC_Pin			//GPIO_PIN_9
#define RST_PORT RST_GPIO_Port	//GPIOA
#define RST_PIN  RST_Pin		//GPIO_PIN_9

/****** TFT DEFINES ******/
//#define ST7735_IS_160X80 1
//#define ST7735_IS_128X128 1
//#define ST7735_IS_160X128_BLUE 1
#define ST7735_IS_160X128_RED 1
#define ST7735_IS_160X128 1
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160

#define DELAY 0x80

#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF822
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define YLWGRN  0xF7E0
#define WHITE   0xFFFF
#define CYANB   0x5EFF
#define color565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define NAVY        	0x000F
#define DARKGREEN   	0x03E0
#define DARKCYAN    	0x03EF
#define MAROON      	0x7800
#define PURPLE      	0x780F
#define OLIVE       	0x7BE0
#define LIGHTGREY   	0xC618
#define DARKGREY    	0x7BEF
//#define RED         	0xF800
#define ORANGE      	0xFD20
#define GREENYELLOW 	0xAFE5
#define PINK        	0xF81F

// call before initializing any SPI devices
void ST7735_Unselect();

void ST7735_Init(uint8_t rotation);
void ST7735_SetRotation(uint8_t m);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ST7735_InvertColors(bool invert);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void  drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void  drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

void drawCourse(int16_t x0, int16_t y0, double angle, uint16_t color);
void drawArrow(int16_t x0, int16_t y0, int16_t r, double angle, int16_t length, uint16_t color1, uint16_t color2);
void drawPosition(int16_t x0, int16_t y0, int16_t r1, double angle, int16_t r2, int16_t tag, uint16_t color);
void drawTrace(int16_t x0, int16_t y0, int16_t r1, double angle, int16_t r2, uint16_t color);
void drawDirection(int16_t x0, int16_t y0, int16_t r, double angle, uint16_t color);
void erasePosition(int16_t x0, int16_t y0, int16_t r1, double angle, int16_t r2);
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

void drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void fillScreen(uint16_t color);

#define 	st7735_SPI 	SPI2

void draw_str_by_rows(uint8_t x, uint8_t y, char *str, FontDef font, uint16_t fg_color, uint16_t bg_color);
void draw_char(uint8_t x, uint8_t y, unsigned char ch, FontDef font, uint16_t fg_color, uint16_t bg_color);
void fill_rectgl(uint8_t x0, uint16_t y0, uint8_t x1, uint16_t y1, uint16_t color);
void fill_screen(uint16_t color);

#endif // __ST7735_H__
