#include "ST7735.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>			//for 'sprintf'

int16_t _width;       ///< Display width as modified by current rotation
int16_t _height;      ///< Display height as modified by current rotation
int16_t cursor_x;     ///< x location to start print()ing text
int16_t cursor_y;     ///< y location to start print()ing text
uint8_t rotation;     ///< Display rotation (0 thru 3)
uint8_t _colstart;   ///< Some displays need this changed to offset
uint8_t _rowstart;       ///< Some displays need this changed to offset
uint8_t _xstart;
uint8_t _ystart;


  const uint8_t
  init_cmds1[] = {            // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color

#if (defined(ST7735_IS_128X128) || defined(ST7735_IS_160X128))
  init_cmds2[] = {            // Init for 7735R, part 2 (1.44" display)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F },           //     XEND = 127
#endif // ST7735_IS_128X128

#ifdef ST7735_IS_160X80
  init_cmds2[] = {            // Init for 7735S, part 2 (160x80 display)
    3,                        //  3 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x4F,             //     XEND = 79
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F ,            //     XEND = 159
    ST7735_INVON, 0 },        //  3: Invert colors
#endif

  init_cmds3[] = {            // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay

void ST7735_Select()
{
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
}

void ST7735_Unselect()
{
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}

void ST7735_Reset()
{
    HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
}

void ST7735_WriteCommand(uint8_t cmd)
 {
    HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ST7735_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
    //HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, &cmd, sizeof(cmd));
 }

void ST7735_WriteData(uint8_t* buff, size_t buff_size)
{
    HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_SET);
    HAL_SPI_Transmit(&ST7735_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
    //HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, buff, buff_size);
    //while(hspi2.State == HAL_SPI_STATE_BUSY_TX);
}

void DisplayInit(const uint8_t *addr)
{
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while(numCommands--) {
        uint8_t cmd = *addr++;
        ST7735_WriteCommand(cmd);

        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & DELAY;
        numArgs &= ~DELAY;
        if(numArgs) {
            ST7735_WriteData((uint8_t*)addr, numArgs);
            addr += numArgs;
        }

        if(ms) {
            ms = *addr++;
            if(ms == 255) ms = 500;
            HAL_Delay(ms);
        }
    }
}

void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // column address set
    ST7735_WriteCommand(ST7735_CASET);
    uint8_t data[] = { 0x00, x0 + _xstart, 0x00, x1 + _xstart };
    ST7735_WriteData(data, sizeof(data));

    // row address set
    ST7735_WriteCommand(ST7735_RASET);
    data[1] = y0 + _ystart;
    data[3] = y1 + _ystart;
    ST7735_WriteData(data, sizeof(data));

    // write to RAM
    ST7735_WriteCommand(ST7735_RAMWR);
}

void ST7735_Init(uint8_t rotation)
{
	// Enable SPI
//	if((st7735_SPI->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
//		SET_BIT(st7735_SPI->CR1, SPI_CR1_SPE);
//	}
	CS_GPIO_Port->BSRR = ( CS_Pin << 16 );

    ST7735_Reset();
    DisplayInit(init_cmds1);
    DisplayInit(init_cmds2);
    DisplayInit(init_cmds3);
#if ST7735_IS_160X80
    _colstart = 24;
    _rowstart = 0;
 /*****  IF Doesn't work, remove the code below (before #elif) *****/
    uint8_t data = 0xC0;
    ST7735_WriteCommand(ST7735_MADCTL);
    ST7735_WriteData(&data,1);

#elif ST7735_IS_128X128
    _colstart = 2;
    _rowstart = 3;
#elif ST7735_IS_160X128_BLUE
    _colstart = 2;
    _rowstart = 1;
#else	//ST7735_IS_160X128_RED
    _colstart = 0;
    _rowstart = 0;
#endif
    ST7735_SetRotation (rotation);
}

void ST7735_SetRotation(uint8_t m)
{
  uint8_t madctl = 0;

  rotation = m % 4; // can't be higher than 3

  switch (rotation)
  {
  case 0:
#if ST7735_IS_160X80
	  madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR;
#else
      madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB;
      _height = ST7735_HEIGHT;
      _width = ST7735_WIDTH;
      _xstart = _colstart;
      _ystart = _rowstart;
#endif
    break;
  case 1:
#if ST7735_IS_160X80
	  madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR;
#else
      madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
      _width = ST7735_HEIGHT;
      _height = ST7735_WIDTH;
    _ystart = _colstart;
    _xstart = _rowstart;
#endif
    break;
  case 2:
#if ST7735_IS_160X80
	  madctl = ST7735_MADCTL_BGR;
#else
      madctl = ST7735_MADCTL_RGB;
      _height = ST7735_HEIGHT;
      _width = ST7735_WIDTH;
    _xstart = _colstart;
    _ystart = _rowstart;
#endif
    break;
  case 3:
#if ST7735_IS_160X80
	  madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR;
#else
      madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
      _width = ST7735_HEIGHT;
      _height = ST7735_WIDTH;
    _ystart = _colstart;
    _xstart = _rowstart;
#endif
    break;
  }

  ST7735_WriteCommand(ST7735_MADCTL);
  ST7735_WriteData(&madctl,1);
}

void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= _width) || (y >= _height))
        return;

    ST7735_SetAddressWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    ST7735_WriteData(data, sizeof(data));
}

void ST7735_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    ST7735_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);

    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                ST7735_WriteData(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                ST7735_WriteData(data, sizeof(data));
            }
        }
    }
}

void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {

    while(*str) {
        if(x + font.width >= _width) {
            x = 0;
            y += font.height;
            if(y + font.height >= _height) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        ST7735_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
}

void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if((x >= _width) || (y >= _height)) return;
    if((x + w - 1) >= _width) w = _width - x;
    if((y + h - 1) >= _height) h = _height - y;

    ST7735_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint8_t data[] = { color >> 8, color & 0xFF };
    HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_SET);
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            HAL_SPI_Transmit(&ST7735_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
}

void ST7735_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if((x >= _width) || (y >= _height)) return;
    if((x + w - 1) >= _width) return;
    if((y + h - 1) >= _height) return;

    ST7735_SetAddressWindow(x, y, x+w-1, y+h-1);
    ST7735_WriteData((uint8_t*)data, sizeof(uint16_t)*w*h);
}

void ST7735_InvertColors(bool invert) {
    ST7735_WriteCommand(invert ? ST7735_INVON : ST7735_INVOFF);
}


void drawPixel(int16_t x, int16_t y, uint16_t color)
{
	ST7735_DrawPixel(x, y, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	ST7735_FillRectangle(x, y, w, h, color);
}


/***********************************************************************************************************/


#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }

#define min(a, b) (((a) < (b)) ? (a) : (b))


void writePixel(int16_t x, int16_t y, uint16_t color)
{
    drawPixel(x, y, color);
}

void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            writePixel(y0, x0, color);
        } else {
            writePixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void  drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	writeLine(x, y, x, y + h - 1, color);
}
void  drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	writeLine(x, y, x + w - 1, y, color);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    if(x0 == x1){
        if(y0 > y1) _swap_int16_t(y0, y1);
        drawFastVLine(x0, y0, y1 - y0 + 1, color);
    } else if(y0 == y1){
        if(x0 > x1) _swap_int16_t(x0, x1);
        drawFastHLine(x0, y0, x1 - x0 + 1, color);
    } else {
        writeLine(x0, y0, x1, y1, color);
    }
}

#include <math.h>
#include "lrns.h"

void drawCourse(int16_t x0, int16_t y0, double angle, uint16_t color)
{
//in void draw_beacons(void)
//void drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, current_device, RED)
}

void drawArrow(int16_t x0, int16_t y0, int16_t r, double angle, int16_t length, uint16_t color1, uint16_t color2)
{
	double xarrow, yarrow, xcentre, ycentre, x1, y1, x2, y2;
	double arrow_angle = angle;		//stay with radian * deg_to_rad;
	double arrow_tail;
	double arrow_sin = sin(arrow_angle);
	double arrow_cos = cos(arrow_angle);

	(angle < M_PI)? (arrow_tail = angle + M_PI): (arrow_tail = angle - M_PI);
//	if(angle < 180) arrow_tail = (angle + 180) * deg_to_rad;
//	else arrow_tail = (angle - 180) * deg_to_rad;
	double arrow_tail_sin = -arrow_sin;		//sin(arrow_tail);
	double arrow_tail_cos = -arrow_cos;		//cos(arrow_tail);

	xarrow = x0 + r * arrow_sin;					//sin(arrow_angle);
	yarrow = y0 - r * arrow_cos;					//cos(arrow_angle);
	x1 = xarrow + 9 * sin(arrow_tail+0.5);
	y1 = yarrow - 9 * cos(arrow_tail+0.5);
	x2 = xarrow + 9 * sin(arrow_tail-0.5);
	y2 = yarrow - 9 * cos(arrow_tail-0.5);
	drawLine(xarrow, yarrow, x1, y1, color1);
	drawLine(xarrow, yarrow, x2, y2, color1);

	x1 = x0 + (r-length) * arrow_sin;				//sin(arrow_angle);
	y1 = y0 - (r-length) * arrow_cos;				//cos(arrow_angle);
	x2 = x0 + (r) * arrow_sin;						//sin(arrow_angle);
	y2 = y0 - (r) * arrow_cos;						//cos(arrow_angle);
	drawLine(x1, y1, x2, y2, color1);

		xcentre = x0 + 1*arrow_cos;					//cos(arrow_angle);
		ycentre = y0 + 1*arrow_sin;					//sin(arrow_angle);
		x1 = xcentre + (r-length) * arrow_sin;		//sin(arrow_angle);
		y1 = ycentre - (r-length) * arrow_cos;		//cos(arrow_angle);
		x2 = xcentre + (r-2) * arrow_sin;			//sin(arrow_angle);
		y2 = ycentre - (r-2) * arrow_cos;			//cos(arrow_angle);
		drawLine(x1, y1, x2, y2, color1);

		xcentre = x0 - 1*arrow_cos;					//cos(arrow_angle);
		ycentre = y0 - 1*arrow_sin;					//sin(arrow_angle);
		x1 = xcentre + (r-length) * arrow_sin;		//sin(arrow_angle);
		y1 = ycentre - (r-length) * arrow_cos;		//cos(arrow_angle);
		x2 = xcentre + (r-2) * arrow_sin;			//sin(arrow_angle);
		y2 = ycentre - (r-2) * arrow_cos;			//cos(arrow_angle);
		drawLine(x1, y1, x2, y2, color1);

	xcentre = x0 + 2*arrow_cos;						//cos(arrow_angle);
	ycentre = y0 + 2*arrow_sin;						//sin(arrow_angle);
	x1 = xcentre + (r-length) * arrow_sin;			//sin(arrow_angle);
	y1 = ycentre - (r-length) * arrow_cos;			//cos(arrow_angle);
	x2 = xcentre + (r-4) * arrow_sin;				//sin(arrow_angle);
	y2 = ycentre - (r-4) * arrow_cos;				//cos(arrow_angle);
	drawLine(x1, y1, x2, y2, color1);

	xcentre = x0 - 2*arrow_cos;						//cos(arrow_angle);
	ycentre = y0 - 2*arrow_sin;						//sin(arrow_angle);
	x1 = xcentre + (r-length) * arrow_sin;			//sin(arrow_angle);
	y1 = ycentre - (r-length) * arrow_cos;			//cos(arrow_angle);
	x2 = xcentre + (r-4) * arrow_sin;				//sin(arrow_angle);
	y2 = ycentre - (r-4) * arrow_cos;				//cos(arrow_angle);
	drawLine(x1, y1, x2, y2, color1);

	xcentre = x0 + 1*arrow_tail_cos;					//cos(arrow_tail);
	ycentre = y0 + 1*arrow_tail_sin;					//sin(arrow_tail);
	x1 = xcentre + (r-length+3) * arrow_tail_sin;		//sin(arrow_tail);
	y1 = ycentre - (r-length+3) * arrow_tail_cos;		//cos(arrow_tail);
	x2 = xcentre + r * arrow_tail_sin;					//sin(arrow_tail);
	y2 = ycentre - r * arrow_tail_cos;					//cos(arrow_tail);
	drawLine(x1, y1, x2, y2, color2);

		xcentre = x0;
		ycentre = y0;
		x1 = xcentre + (r-length+3) * arrow_tail_sin;	//sin(arrow_tail);
		y1 = ycentre - (r-length+3) * arrow_tail_cos;	//cos(arrow_tail);
		x2 = xcentre + r * arrow_tail_sin;				//sin(arrow_tail);
		y2 = ycentre - r * arrow_tail_cos;				//cos(arrow_tail);
		drawLine(x1, y1, x2, y2, color2);

	xcentre = x0 - 1*arrow_tail_cos;					//cos(arrow_tail);
	ycentre = y0 - 1*arrow_tail_sin;					//sin(arrow_tail);
	x1 = xcentre + (r-length+3) * arrow_tail_sin;		//sin(arrow_tail);
	y1 = ycentre - (r-length+3) * arrow_tail_cos;		//cos(arrow_tail);
	x2 = xcentre + r * arrow_tail_sin;					//sin(arrow_tail);
	y2 = ycentre - r * arrow_tail_cos;					//cos(arrow_tail);
	drawLine(x1, y1, x2, y2, color2);

}

void drawPosition(int16_t x0, int16_t y0, int16_t r1, double angle, int16_t r2, int16_t tag, uint16_t color)
{
	int16_t xcentre, ycentre;
//	double pos_angle = angle * deg_to_rad;
	char Line[18][18];

	xcentre = x0 + r1 * sin(angle);
	ycentre = y0 - r1 * cos(angle);

	sprintf(&Line[0][0], "%01d", tag);
	ST7735_WriteString(xcentre-2, ycentre-3, &Line[0][0], Font_6x8, color,BLACK);

	drawCircle(xcentre, ycentre, r2, color);
}
void drawTrace(int16_t x0, int16_t y0, int16_t r1, double angle, int16_t r2, uint16_t color)
{
	int16_t xcentre, ycentre;
//	double pos_angle = angle * deg_to_rad;

	xcentre = x0 + r1 * sin(angle);
	ycentre = y0 - r1 * cos(angle);

	drawCircle(xcentre, ycentre, r2, color);
}
void drawDirection(int16_t x0, int16_t y0, int16_t r, double angle, uint16_t color)
{
	int16_t x1, y1;
	//	double pos_angle = angle * deg_to_rad;

	x1 = x0 + r * sin(angle);
	y1 = y0 - r * cos(angle);

	writeLine(x0, y0, x1, y1, color);
}
void erasePosition(int16_t x0, int16_t y0, int16_t r1, double angle, int16_t r2)
{
	int16_t xcentre = x0 + r1 * sin(angle);
	int16_t ycentre = y0 - r1 * cos(angle);

    drawFastVLine(xcentre, ycentre-r2, 2*r2+1, BLACK);
    fillCircleHelper(xcentre, ycentre, r2, 3, 0, BLACK);
}

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    writePixel(x0  , y0+r, color);
    writePixel(x0  , y0-r, color);
    writePixel(x0+r, y0  , color);
    writePixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        writePixel(x0 + x, y0 + y, color);
        writePixel(x0 - x, y0 + y, color);
        writePixel(x0 + x, y0 - y, color);
        writePixel(x0 - x, y0 - y, color);
        writePixel(x0 + y, y0 + x, color);
        writePixel(x0 - y, y0 + x, color);
        writePixel(x0 + y, y0 - x, color);
        writePixel(x0 - y, y0 - x, color);
    }
}

void drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            writePixel(x0 + x, y0 + y, color);
            writePixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            writePixel(x0 + x, y0 - y, color);
            writePixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            writePixel(x0 - y, y0 + x, color);
            writePixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            writePixel(x0 - y, y0 - x, color);
            writePixel(x0 - x, y0 - y, color);
        }
    }
}

void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color)
{

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;
    int16_t px    = x;
    int16_t py    = y;

    delta++; // Avoid some +1's in the loop

    while(x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        // These checks avoid double-drawing certain lines, important
        // for the SSD1306 library which has an INVERT drawing mode.
        if(x < (y + 1)) {
            if(corners & 1) drawFastVLine(x0+x, y0-y, 2*y+delta, color);
            if(corners & 2) drawFastVLine(x0-x, y0-y, 2*y+delta, color);
        }
        if(y != py) {
            if(corners & 1) drawFastVLine(x0+py, y0-px, 2*px+delta, color);
            if(corners & 2) drawFastVLine(x0-py, y0-px, 2*px+delta, color);
            py = y;
        }
        px = x;
    }
}

void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    drawFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}



void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y+h-1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x+w-1, y, h, color);
}

void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if(r > max_radius) r = max_radius;
    // smarter version
    drawFastHLine(x+r  , y    , w-2*r, color); // Top
    drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    drawFastVLine(x    , y+r  , h-2*r, color); // Left
    drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    drawCircleHelper(x+r    , y+r    , r, 1, color);
    drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}


void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if(r > max_radius) r = max_radius;
    // smarter version
    fillRect(x+r, y, w-2*r, h, color);
    // draw four corners
    fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}


void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}


void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        drawFastHLine(a, y0, b-a+1, color);
        return;
    }

    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
    int32_t
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        drawFastHLine(a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        drawFastHLine(a, y, b-a+1, color);
    }
}

void fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

void ST7735_FillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

#define CHAR_LEN 198                 // 11*18  i.e. width * height

volatile uint16_t char_buffer[CHAR_LEN];

static void sendCmd(uint8_t Cmd);
static void sendData(uint8_t Data );
static void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
static void columnSet(uint8_t ColumnStart, uint8_t ColumnEnd);
static void rowSet(uint8_t RowStart, uint8_t RowEnd);
static void char_to_buffer(uint8_t x_pos, unsigned char ch, FontDef font, uint16_t fg_color, uint16_t bg_color);

void draw_str_by_rows(uint8_t x, uint8_t y, char* str, FontDef font, uint16_t fg_color, uint16_t bg_color) {
//	static uint16_t string_buff[ST7735_WIDTH * 18];
	uint16_t string_buff[ST7735_WIDTH * font.height];
	uint16_t pixel_index = 0;		//from 0 to BUF_LEN max
    uint16_t row_in_char = 0;		//row in a particular character
    uint8_t length = 0;

    for(uint8_t i=0; str[i]!='\0'; i++)
        {
    		length++;
        }

    for (uint8_t row_in_str = 0; row_in_str < font.height; row_in_str++)	//successive enumeration rows in string (0 to FONT_11x18_HEIGHT-1)
    {
        for(uint8_t char_ind=0; char_ind<length; char_ind++)					//successive enumeration characters in string (0 to length-1)
        {
        	row_in_char = font.data[((uint16_t)str[char_ind] - 32)*font.height + row_in_str];

        	for (uint8_t j=0; j<font.width;j++)		//filling pixels in a particular row of each character
        	{
        		string_buff[pixel_index++]=(row_in_char & (uint16_t)0x8000)? fg_color: bg_color;
        		row_in_char = row_in_char<< 1;
        	}
		}
	}

    setWindow(x, y, (x + (font.width * length) - 1), (y + font.height - 1));

    for (uint16_t i=0; i<(font.width * font.height * length); i++)
    {
        while (!(st7735_SPI->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&st7735_SPI->DR) = (string_buff[i] >> 8);
        while (!(st7735_SPI->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&st7735_SPI->DR) = (string_buff[i] & 0xFF);
    }
    while (!(st7735_SPI->SR & SPI_SR_TXE) || (st7735_SPI->SR & SPI_SR_BSY));
}

void draw_char(uint8_t x, uint8_t y, unsigned char ch, FontDef font, uint16_t fg_color, uint16_t bg_color) {

    char_to_buffer(x, ch, font, fg_color,bg_color);

    setWindow(x, y, (x + font.width - 1), (y + font.height - 1));

    for (uint16_t i=0; i<(font.width * font.height); i++)
    {
           while (!(st7735_SPI->SR & SPI_SR_TXE));
           *((__IO uint8_t *)&st7735_SPI->DR) = (char_buffer[i] >> 8);
           while (!(st7735_SPI->SR & SPI_SR_TXE));
           *((__IO uint8_t *)&st7735_SPI->DR) = (char_buffer[i] & 0xFF);
    }
    while (!(st7735_SPI->SR & SPI_SR_TXE) || (st7735_SPI->SR & SPI_SR_BSY));
}

void fill_screen(uint16_t color)
{
	fill_rectgl(0, 0,  ST7735_WIDTH, ST7735_HEIGHT, color);
}

void fill_rectgl(uint8_t x0, uint16_t y0, uint8_t x1, uint16_t y1, uint16_t color)
{
	setWindow(x0, y0, x1-1, y1-1);

   uint32_t len = (uint32_t)((x1-x0)*(y1-y0));

   for (uint32_t i=0; i < len; i++)
   {
          while (!(st7735_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&st7735_SPI->DR) = (color >> 8);
          while (!(st7735_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&st7735_SPI->DR) = (color & 0xFF);
   }
   while (!(st7735_SPI->SR & SPI_SR_TXE) || (st7735_SPI->SR & SPI_SR_BSY));
}

// static functions
//==============================================================================
static void char_to_buffer(uint8_t x_pos, unsigned char ch, FontDef font, uint16_t fg_color, uint16_t bg_color) {

    uint16_t raw_index = (((uint16_t)ch - 32) * font.height);	//raw index in font array

    for (uint8_t i = 0; i < font.height; i++)
    {
        uint16_t raw = font.data[raw_index+i];				//raw in character
        uint16_t pixel= (i*font.width);						//color of pixel

        for (uint8_t j=0; j<font.width;j++) {

            char_buffer[pixel++]=(raw & (uint16_t)0x8000)? fg_color: bg_color;
            raw = raw << 1;
        }
    }
}

static void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	columnSet(x0, x1);
	rowSet(y0, y1);
	// write to RAM
	sendCmd(ST7735_RAMWR);
}

static void columnSet(uint8_t ColumnStart, uint8_t ColumnEnd){

  if (ColumnStart > ColumnEnd) return;
  if (ColumnEnd > ST7735_WIDTH) return;

  ColumnStart += _colstart;
  ColumnEnd += _colstart;

  sendCmd(ST7735_CASET);
  sendData(ColumnStart >> 8);
  sendData(ColumnStart & 0xFF);
  sendData(ColumnEnd >> 8);
  sendData(ColumnEnd & 0xFF);
}

static void rowSet(uint8_t RowStart, uint8_t RowEnd){

  if (RowStart > RowEnd) return;
  if (RowEnd > ST7735_HEIGHT) return;

  RowStart += _rowstart;
  RowEnd += _rowstart;

  sendCmd(ST7735_RASET);
  sendData(RowStart >> 8);
  sendData(RowStart & 0xFF);
  sendData(RowEnd >> 8);
  sendData(RowEnd & 0xFF);
}

static void sendCmd(uint8_t Cmd){
	// pin DC LOW
	DC_GPIO_Port->BSRR = ( DC_Pin << 16 );
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (st7735_SPI->SR & SPI_SR_TXE) == RESET ){};
	// заполняем буфер передатчика 1 байт информации--------------
	*((__IO uint8_t *)&st7735_SPI->DR) = Cmd;
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (st7735_SPI->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
	// pin DC HIGH
	DC_GPIO_Port->BSRR = DC_Pin;
}

static void sendData(uint8_t Data ){
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (st7735_SPI->SR & SPI_SR_TXE) == RESET ){};
	// передаем 1 байт информации--------------
	*((__IO uint8_t *)&st7735_SPI->DR) = Data;
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (st7735_SPI->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
}
