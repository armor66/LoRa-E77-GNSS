#include <math.h>
#include "lcd_display.h"

#define LCD_WIDTH		128
#define LCD_HEIGHT		160

#define _swap_int16_t(a, b)	\
{							\
    int16_t t = a;			\
    a = b;					\
    b = t;					\
}

//__inline:
static void sendCmd(uint8_t Cmd);
static void sendData(uint8_t Data );
static void sendDataMass(uint8_t* buff, size_t buff_size);
static void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
static void drawPixel(uint8_t x, uint8_t y, uint16_t color);
static void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
static void drawVLine(uint8_t x, uint8_t y, uint8_t h, uint16_t color);
static void drawHLine(uint8_t x, uint8_t y, uint8_t w, uint16_t color);
static void fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t corners, uint8_t delta, uint16_t color);
static void fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);

#ifdef ST7735

const uint8_t
init_cmds1[] = {            // Init for 7735R, part 1 (red or green tab)
	15,                       // 15 commands in list:
	SWRESET,   ST7735_DELAY,  //  1: Software reset, 0 args, w/delay
    	150,                    //     150 ms delay
	SLPOUT ,   ST7735_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
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
	INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
	COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
		0x05 },                 //     16-bit color

init_cmds2[] = {            // Init for 7735R, part 2 (1.44" display)
	2,                        //  2 commands in list:
	CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
		0x00, 0x00,             //     XSTART = 0
		0x00, 0x7F,             //     XEND = 127
    RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
		0x00, 0x00,             //     XSTART = 0
		0x00, 0x7F },           //     XEND = 127

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
	ST7735_NORON  ,    ST7735_DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
	ST7735_DISPON ,    ST7735_DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay

void displayInit(const uint8_t *addr)
{
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while(numCommands--) {
        uint8_t cmd = *addr++;
        HAL_Delay(1);
        DC_GPIO_Port->BSRR = (DC_Pin << 16);	// pin DC LOW
        HAL_SPI_Transmit(&ST7735_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
    	DC_GPIO_Port->BSRR = DC_Pin;			// pin DC HIGH
        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & ST7735_DELAY;
        numArgs &= ~ST7735_DELAY;
        if(numArgs) {

        	sendDataMass((uint8_t*)addr, numArgs);
            addr += numArgs;
        }

        if(ms) {
            ms = *addr++;
            if(ms == 255) ms = 500;
            HAL_Delay(ms);
        }
    }
}
void setST7735rotation(uint8_t rotation)
{
	  uint8_t madctl = 0;

	  switch (rotation)
	  {
	  case 0:
	      madctl = MADCTL_MX | MADCTL_MY | MADCTL_RGB;
	      break;
	  case 1:
	      madctl = MADCTL_MY | MADCTL_MV | MADCTL_RGB;
	      break;
	  case 2:
	      madctl = MADCTL_RGB;
	      break;
	  case 3:
	      madctl = MADCTL_MX | MADCTL_MV | MADCTL_RGB;
	      break;
	  }

      sendCmd(MADCTL);
	  sendData(madctl);
	fill_screen(BLACK);
}

void st7735_init(uint8_t rotation)
{
	CS_GPIO_Port->BSRR = ( CS_Pin << 16 );

	RST_GPIO_Port->BSRR = (RST_Pin << 16);
	HAL_Delay(50);	//20
	RST_GPIO_Port->BSRR = RST_Pin;

    displayInit(init_cmds1);
    displayInit(init_cmds2);
    displayInit(init_cmds3);

//#if ST7735_IS_160X128_BLUE
//    _colstart = 2;
//    _rowstart = 1;
//#else ST7735_IS_160X128_RED
//    _colstart = 0;
//    _rowstart = 0;
//#endif
   setST7735rotation (rotation);
}

#else	//NV3023 display

static void setRotation(void);

void nv3023_init(void)
{
	// Enable SPI
	if((LCD_SPI->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
		// If disabled, I enable it
		SET_BIT(LCD_SPI->CR1, SPI_CR1_SPE);	// NV3023_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
	}
	CS_GPIO_Port->BSRR = ( CS_Pin << 16 );	//select();

	RST_GPIO_Port->BSRR = RST_Pin;			//HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
	HAL_Delay(20);
	RST_GPIO_Port->BSRR = (RST_Pin << 16);	//HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(120);
	RST_GPIO_Port->BSRR = RST_Pin;			//HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
	HAL_Delay(200);
//128x128							//0.96"
sendCmd(0xff);sendData(0xa5);	//0xFF, 0xA5,
sendCmd(0x3E);sendData(0x09);	//0x3E, 0x09,	my	mx	mv	ml	rgb
sendCmd(0x3A);sendData(0x65);	//0x3A, 0x65,
sendCmd(0x82);sendData(0x00);	//0x82, 0x00,
sendCmd(0x98);sendData(0x00);	//0x98, 0x00,
sendCmd(0x63);sendData(0x0f);	//0x63, 0x0F,
sendCmd(0x64);sendData(0x0f);	//0x64, 0x0F,
sendCmd(0xB4);sendData(0x24);	//0xB4, 0x34,!!!
sendCmd(0xB5);sendData(0x30);	//0xB5, 0x30,
sendCmd(0x83);sendData(0x03);	//0x83, 0x03,
sendCmd(0x86);sendData(0x04);	//0x86, 0x04,		frc
sendCmd(0x87);sendData(0x16);	//0x87, 0x16,		16
sendCmd(0x88);sendData(0x0A);	//0x88, 0x2f,!!!	VCOM
sendCmd(0x89);sendData(0x27);	//0x89, 0x29,!!!	2F
sendCmd(0x93);sendData(0x63);	//0x93, 0x63,
sendCmd(0x96);sendData(0x81);	//0x96, 0x81,
sendCmd(0xC3);sendData(0x10);	//0xC3, 0x11,!!!
sendCmd(0xE6);sendData(0x00);	//0xE6, 0x00,
sendCmd(0x99);sendData(0x01);	//!!!

////////////////////////gamma_set_ips//////////////////////////////////////
sendCmd(0x70);sendData(0x09);	//0x70, 0x07,	 [3:0]VRP 31 1
sendCmd(0x71);sendData(0x1D);	//0x71, 0x21,	 [5:0]VRP 30 3
sendCmd(0x72);sendData(0x14);	//0x72, 0x15,	 [3:0]VRP 29 7
sendCmd(0x73);sendData(0x0A);	//0x73, 0x0F,	 [4:0]VRP 28 9
sendCmd(0x74);sendData(0x11);	//0x74, 0x16,	 [4:0]VRP 25 11
sendCmd(0x75);sendData(0x16);	//0x75, 0x18,	 [3:0]VRP 23 13
sendCmd(0x76);sendData(0x38);	//0x76, 0x2e,	 [6:0]VRP 21 5
sendCmd(0x77);sendData(0x0B);	//0x77, 0x0A,	 [3:0]VRP 17 15
sendCmd(0x78);sendData(0x08);	//0x78, 0x07,	 [3:0]VRP 14 16
sendCmd(0x79);sendData(0x3E);	//0x79, 0x3A,	 [6:0]VRP 10 6
sendCmd(0x7a);sendData(0x07);	//0x7A, 0x04,	 [3:0]VRP 8 14
sendCmd(0x7b);sendData(0x0D);	//0x7B, 0x09,	 [4:0]VRP 6 12
sendCmd(0x7c);sendData(0x16);	//0x7C, 0x0E,	 [4:0]VRP 3 10
sendCmd(0x7d);sendData(0x0F);	//0x7D, 0x1b,	 [3:0]VRP 2 8
sendCmd(0x7e);sendData(0x14);	//0x7E, 0x20,	 [5:0]VRP 1 4
sendCmd(0x7f);sendData(0x05);	//0x7F, 0x08,	 [3:0]VRP 0 2

sendCmd(0xa0);sendData(0x04);	//0xA0, 0x30,	 [3:0]VRN 31 1
sendCmd(0xa1);sendData(0x28);	//0xA1, 0x2F,	 [5:0]VRN 30 3
sendCmd(0xa2);sendData(0x0C);	//0xA2, 0x0D,	 [3:0]VRN 29 7
sendCmd(0xa3);sendData(0x11);	//0xA3, 0x13,	 [4:0]VRN 28 9
sendCmd(0xa4);sendData(0x0B);	//0xA4, 0x0D,	 [4:0]VRN 25 11
sendCmd(0xa5);sendData(0x23);	//0xA5, 0x23,	 [3:0]VRN 23 13
sendCmd(0xa6);sendData(0x45);	//0xA6, 0x42,	 [6:0]VRN 21 5
sendCmd(0xa7);sendData(0x07);	//0xA7, 0x07,	 [3:0]VRN 17 15
sendCmd(0xa8);sendData(0x0A);	//0xA8, 0x09,	 [3:0]VRN 14 16
sendCmd(0xa9);sendData(0x3B);	//0xA9, 0x3D,	 [6:0]VRN 10 6
sendCmd(0xaa);sendData(0x0D);	//0xAA, 0x09,	 [3:0]VRN 8 14
sendCmd(0xab);sendData(0x18);	//0xAB, 0x12,	 [4:0]VRN 6 12
sendCmd(0xac);sendData(0x14);	//0xAC, 0x09,	 [4:0]VRN 3 10
sendCmd(0xad);sendData(0x0F);	//0xAD, 0x04,	 [3:0]VRN 2 8
sendCmd(0xae);sendData(0x19);	//0xAE, 0x29,	 [5:0]VRN 1 4
sendCmd(0xaf);sendData(0x08);	//0xAF, 0x07,	 [3:0]VRN 0 2
//////////////////////////////////////////////////////////////////

sendCmd(0xff);sendData(0x00);	//0xFF, 0x00,
sendCmd(INVON);
sendCmd(0x11);HAL_Delay(120);	//0x11,
//sendCmd(0x36);sendData(0x08);	//!commented!
sendCmd(0x29);					//0x29
HAL_Delay(20);

setRotation();
}

static void setRotation(void)
{
	sendCmd(MADCTL);
	sendData(MADCTL_MY | MADCTL_BGR);
	fill_screen(BLACK);
}
#endif	//choose display

void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
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
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
    if(x0 == x1){
        if(y0 > y1) _swap_int16_t(y0, y1);
        drawVLine(x0, y0, y1 - y0 + 1, color);
    } else if(y0 == y1){
        if(x0 > x1) _swap_int16_t(x0, x1);
        drawHLine(x0, y0, x1 - x0 + 1, color);
    } else {
        drawLine(x0, y0, x1, y1, color);
    }
}

void draw_course(uint8_t x0, uint8_t y0, double angle, uint16_t color)
{
//in void draw_beacons(void)
//void drawPosition(63, 97, scaled_dist, azimuth_relative_rad, 7, current_device, RED)
}

void draw_arrow(uint8_t x0, uint8_t y0, uint8_t r, double angle, uint8_t length, uint16_t color1, uint16_t color2)
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
	draw_line(xarrow, yarrow, x1, y1, color1);
	draw_line(xarrow, yarrow, x2, y2, color1);

	x1 = x0 + (r-length) * arrow_sin;				//sin(arrow_angle);
	y1 = y0 - (r-length) * arrow_cos;				//cos(arrow_angle);
	x2 = x0 + (r) * arrow_sin;						//sin(arrow_angle);
	y2 = y0 - (r) * arrow_cos;						//cos(arrow_angle);
	draw_line(x1, y1, x2, y2, color1);

		xcentre = x0 + 1*arrow_cos;					//cos(arrow_angle);
		ycentre = y0 + 1*arrow_sin;					//sin(arrow_angle);
		x1 = xcentre + (r-length) * arrow_sin;		//sin(arrow_angle);
		y1 = ycentre - (r-length) * arrow_cos;		//cos(arrow_angle);
		x2 = xcentre + (r-2) * arrow_sin;			//sin(arrow_angle);
		y2 = ycentre - (r-2) * arrow_cos;			//cos(arrow_angle);
		draw_line(x1, y1, x2, y2, color1);

		xcentre = x0 - 1*arrow_cos;					//cos(arrow_angle);
		ycentre = y0 - 1*arrow_sin;					//sin(arrow_angle);
		x1 = xcentre + (r-length) * arrow_sin;		//sin(arrow_angle);
		y1 = ycentre - (r-length) * arrow_cos;		//cos(arrow_angle);
		x2 = xcentre + (r-2) * arrow_sin;			//sin(arrow_angle);
		y2 = ycentre - (r-2) * arrow_cos;			//cos(arrow_angle);
		draw_line(x1, y1, x2, y2, color1);

	xcentre = x0 + 2*arrow_cos;						//cos(arrow_angle);
	ycentre = y0 + 2*arrow_sin;						//sin(arrow_angle);
	x1 = xcentre + (r-length) * arrow_sin;			//sin(arrow_angle);
	y1 = ycentre - (r-length) * arrow_cos;			//cos(arrow_angle);
	x2 = xcentre + (r-4) * arrow_sin;				//sin(arrow_angle);
	y2 = ycentre - (r-4) * arrow_cos;				//cos(arrow_angle);
	draw_line(x1, y1, x2, y2, color1);

	xcentre = x0 - 2*arrow_cos;						//cos(arrow_angle);
	ycentre = y0 - 2*arrow_sin;						//sin(arrow_angle);
	x1 = xcentre + (r-length) * arrow_sin;			//sin(arrow_angle);
	y1 = ycentre - (r-length) * arrow_cos;			//cos(arrow_angle);
	x2 = xcentre + (r-4) * arrow_sin;				//sin(arrow_angle);
	y2 = ycentre - (r-4) * arrow_cos;				//cos(arrow_angle);
	draw_line(x1, y1, x2, y2, color1);

	xcentre = x0 + 1*arrow_tail_cos;					//cos(arrow_tail);
	ycentre = y0 + 1*arrow_tail_sin;					//sin(arrow_tail);
	x1 = xcentre + (r-length+3) * arrow_tail_sin;		//sin(arrow_tail);
	y1 = ycentre - (r-length+3) * arrow_tail_cos;		//cos(arrow_tail);
	x2 = xcentre + r * arrow_tail_sin;					//sin(arrow_tail);
	y2 = ycentre - r * arrow_tail_cos;					//cos(arrow_tail);
	draw_line(x1, y1, x2, y2, color2);

		xcentre = x0;
		ycentre = y0;
		x1 = xcentre + (r-length+3) * arrow_tail_sin;	//sin(arrow_tail);
		y1 = ycentre - (r-length+3) * arrow_tail_cos;	//cos(arrow_tail);
		x2 = xcentre + r * arrow_tail_sin;				//sin(arrow_tail);
		y2 = ycentre - r * arrow_tail_cos;				//cos(arrow_tail);
		draw_line(x1, y1, x2, y2, color2);

	xcentre = x0 - 1*arrow_tail_cos;					//cos(arrow_tail);
	ycentre = y0 - 1*arrow_tail_sin;					//sin(arrow_tail);
	x1 = xcentre + (r-length+3) * arrow_tail_sin;		//sin(arrow_tail);
	y1 = ycentre - (r-length+3) * arrow_tail_cos;		//cos(arrow_tail);
	x2 = xcentre + r * arrow_tail_sin;					//sin(arrow_tail);
	y2 = ycentre - r * arrow_tail_cos;					//cos(arrow_tail);
	draw_line(x1, y1, x2, y2, color2);

}

void draw_circle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0  , y0+r, color);
    drawPixel(x0  , y0-r, color);
    drawPixel(x0+r, y0  , color);
    drawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

//==============================================================================
/*new guess more fast functions*/
//==============================================================================
void draw_trace(uint8_t x0, uint8_t y0, uint8_t r1, double angle, uint8_t r2, uint16_t color)
{
	uint8_t xcentre, ycentre;
//	double pos_angle = angle * deg_to_rad;

	xcentre = x0 + r1 * sin(angle);
	ycentre = y0 - r1 * cos(angle);

//	fillCircle(xcentre, ycentre, r2, color);
	draw_circle(xcentre, ycentre, r2, color);
	draw_circle(xcentre, ycentre, (r2 - 1), color);
}

void draw_direction(uint8_t x0, uint8_t y0, uint8_t r, double angle, uint16_t color)
{
	uint8_t x1, y1;
	//	double pos_angle = angle * deg_to_rad;

	x1 = x0 + r * sin(angle);
	y1 = y0 - r * cos(angle);

	draw_line(x0, y0, x1, y1, color);
}

void draw_position(uint8_t x0, uint8_t y0, uint8_t r1, double angle, uint8_t r2, uint8_t tag, uint16_t color)
{
	uint8_t xcentre, ycentre;
//	double pos_angle = angle * deg_to_rad;
	tag += 48;
	uint8_t *p = &tag;

	xcentre = x0 + r1 * sin(angle);
	ycentre = y0 - r1 * cos(angle);

	draw_char(xcentre-2, ycentre-3, *p, &Font_6x8, color,BLACK);
	draw_circle(xcentre, ycentre, r2, color);
}

void erase_position(uint8_t x0, uint8_t y0, uint8_t r1, double angle, uint8_t r2)
{
	uint8_t xcentre = x0 + r1 * sin(angle);
	uint8_t ycentre = y0 - r1 * cos(angle);
//fill circle:
	fillCircle(xcentre, ycentre, r2, BLACK);
//    drawVLine(xcentre, ycentre-r2, 2*r2+1, BLACK);
//    fillCircleHelper(xcentre, ycentre, r2, 3, 0, BLACK);
}

void draw_str_by_rows(uint8_t x, uint8_t y, char* str, FontDef_t* Font, uint16_t color, uint16_t bg_color) {

	uint16_t string_buff[Font->CharsMax * Font->FontPexels];
	uint16_t pixel_index = 0;		//from 0 to BUF_LEN max
    uint16_t row_in_char = 0;		//row in a particular character
    uint8_t length = 0;

    for(uint8_t i=0; str[i]!='\0'; i++)
        {
    		length++;
        }

    for (uint8_t row_in_str = 0; row_in_str < Font->FontHeight; row_in_str++)	//successive enumeration rows in string (0 to FONT_11x18_HEIGHT-1)
    {
        for(uint8_t char_ind=0; char_ind<length; char_ind++)					//successive enumeration characters in string (0 to length-1)
        {
        	row_in_char = Font->data[((uint16_t)str[char_ind] - 32)*Font->FontHeight + row_in_str];

        	for (uint8_t j=0; j<Font->FontWidth;j++)		//filling pixels in a particular row of each character
        	{
        		string_buff[pixel_index++]=(row_in_char & (uint16_t)0x8000)? color: bg_color;
        		row_in_char = row_in_char<< 1;
        	}
		}
	}

    setWindow(x, y, (x + (Font->FontWidth * length) - 1), (y + Font->FontHeight - 1));

    for (uint16_t i=0; i<(Font->FontPexels * length); i++)
    {
        while (!(LCD_SPI->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&LCD_SPI->DR) = (string_buff[i] >> 8);
        while (!(LCD_SPI->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&LCD_SPI->DR) = (string_buff[i] & 0xFF);
    }
    while (!(LCD_SPI->SR & SPI_SR_TXE) || (LCD_SPI->SR & SPI_SR_BSY));
}

void draw_char(uint8_t x, uint8_t y, char ch, FontDef_t *Font, uint16_t color, uint16_t bg_color)
{
    uint32_t i, b, j;

    setWindow(x, y, (x + Font->FontWidth - 1), (y + Font->FontHeight - 1));

    for(i = 0; i < Font->FontHeight; i++) {
        b = Font->data[(ch - 32) * Font->FontHeight + i];
        for(j = 0; j < Font->FontWidth; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                sendDataMass(data, sizeof(data));
            } else {
                uint8_t data[] = { bg_color >> 8, bg_color & 0xFF };
                sendDataMass(data, sizeof(data));
            }
        }
    }
}

void fill_screen(uint16_t color)
{
	fill_rectgl(0, 0,  LCD_WIDTH, LCD_HEIGHT, color);
}

void fill_rectgl(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
	setWindow(x0, y0, x1-1, y1-1);

   uint32_t len = (uint32_t)((x1-x0)*(y1-y0));

   for (uint32_t i=0; i < len; i++)
   {
          while (!(LCD_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&LCD_SPI->DR) = (color >> 8);
          while (!(LCD_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&LCD_SPI->DR) = (color & 0xFF);
   }
   while (!(LCD_SPI->SR & SPI_SR_TXE) || (LCD_SPI->SR & SPI_SR_BSY));
}
//==============================================================================
/*static __inline functions*/
//==============================================================================
__inline static void sendCmd(uint8_t Cmd)
{
	DC_GPIO_Port->BSRR = (DC_Pin << 16);	// pin DC LOW
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (LCD_SPI->SR & SPI_SR_TXE) == RESET ){};
	// заполняем буфер передатчика 1 байт информации--------------
	*((__IO uint8_t *)&LCD_SPI->DR) = Cmd;
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (LCD_SPI->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};

	DC_GPIO_Port->BSRR = DC_Pin;			// pin DC HIGH
}

__inline static void sendData(uint8_t Data )
{
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (LCD_SPI->SR & SPI_SR_TXE) == RESET ){};
	// передаем 1 байт информации--------------
	*((__IO uint8_t *)&LCD_SPI->DR) = Data;
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (LCD_SPI->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
}	//NV3030B_SendData

__inline static void sendDataMass(uint8_t* buff, size_t buff_size)
{
	while( buff_size )
	{
		// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
		while( (LCD_SPI->SR & SPI_SR_TXE) == RESET ){};
		// передаем 1 байт информации--------------
		*((__IO uint8_t *)&LCD_SPI->DR) = *buff++;
		buff_size--;
	}
	// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
	while( (LCD_SPI->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
}

__inline static void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // column address set
	sendCmd(CASET);
	uint8_t data[] = {0x00, x0, 0x00, x1};
	sendDataMass(data, sizeof(data));
	// row address set
	sendCmd(RASET);
    data[1] = y0;
    data[3] = y1;
    sendDataMass(data, sizeof(data));
    // write to RAM
    sendCmd(RAMWR);
}

__inline static void drawPixel(uint8_t x, uint8_t y, uint16_t color)
{
    if((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;

    setWindow(x, y, x, y);
    uint8_t data[] = { color >> 8, color & 0xFF };
    sendDataMass(data, sizeof(data));
}

__inline static void drawVLine(uint8_t x, uint8_t y, uint8_t h, uint16_t color)
{
	setWindow(x, y, x, y + h - 1);

   for (uint16_t i=0; i < h; i++)
   {
          while (!(LCD_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&LCD_SPI->DR) = (color >> 8);
          while (!(LCD_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&LCD_SPI->DR) = (color & 0xFF);
   }
   while (!(LCD_SPI->SR & SPI_SR_TXE) || (LCD_SPI->SR & SPI_SR_BSY));
}

__inline static void drawHLine(uint8_t x, uint8_t y, uint8_t w, uint16_t color)
{
	setWindow(x, y, x + w - 1, y);

   for (uint16_t i=0; i < w; i++)
   {
          while (!(LCD_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&LCD_SPI->DR) = (color >> 8);
          while (!(LCD_SPI->SR & SPI_SR_TXE));
          *((__IO uint8_t *)&LCD_SPI->DR) = (color & 0xFF);
   }
   while (!(LCD_SPI->SR & SPI_SR_TXE) || (LCD_SPI->SR & SPI_SR_BSY));
}

__inline void fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t corners, uint8_t delta, uint16_t color)
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
            if(corners & 1) drawVLine(x0+x, y0-y, 2*y+delta, color);
            if(corners & 2) drawVLine(x0-x, y0-y, 2*y+delta, color);
        }
        if(y != py) {
            if(corners & 1) drawVLine(x0+py, y0-px, 2*px+delta, color);
            if(corners & 2) drawVLine(x0-py, y0-px, 2*px+delta, color);
            py = y;
        }
        px = x;
    }
}

__inline static void fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color)
{
    drawVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

