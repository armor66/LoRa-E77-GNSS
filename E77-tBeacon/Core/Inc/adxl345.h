#ifndef __ADXL345_H
#define __ADXL345_H

#include "i2c.h"

void i2c_init(void);
void i2c_clock_disable(void);
void i2c_clock_enable(void);
//uint8_t i2c_poll(uint8_t i2c_addr);
//void i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data);
//uint8_t i2c_read(uint8_t i2c_addr, uint8_t reg_addr);
//void i2c_read_multiple(uint8_t i2c_addr, uint8_t reg_addr, uint8_t size, uint8_t *buffer);

struct AdxlCommands
{
	uint8_t	DEVID;	/*	Device ID	*/
	uint8_t	THRESH_TAP;	/*	Tap threshold	*/
	uint8_t	OFSX;	/*	X-axis offset	*/
	uint8_t	OFSY;	/*	Y-axis offset	*/
	uint8_t	OFSZ;	/*	Z-axis offset	*/
	uint8_t	DUR;	/*	Tap duration	*/
	uint8_t	Latent;	/*	Tap latency	*/
	uint8_t	Window;	/*	Tap window	*/
	uint8_t	THRESH_ACT;	/*	Activity threshold	*/
	uint8_t	THRESH_INACT;	/*	Inactivity threshold	*/
	uint8_t	TIME_INACT;	/*	Inactivity time	*/
	uint8_t	ACT_INACT_CTL;	/*	Axis enable control for activity and inactivity detection	*/
	uint8_t	THRESH_FF;	/*	Free-fall threshold	*/
	uint8_t	TIME_FF;	/*	Free-fall time	*/
	uint8_t	TAP_AXES;	/*	Axis control for single tap/double tap	*/
	uint8_t	ACT_TAP_STATUS;	/*	Source of single tap/double tap	*/
	uint8_t	BW_RATE;	/*	Data rate and power mode control	*/
	uint8_t	POWER_CTL;	/*	Power-saving features control	*/
	uint8_t	INT_ENABLE;	/*	Interrupt enable control	*/
	uint8_t	INT_MAP;	/*	Interrupt mapping control	*/
	uint8_t	INT_SOURCE;	/*	Source of interrupts	*/
	uint8_t	DATA_FORMAT;	/*	Data format control	*/
	uint8_t	DATAX0;	/*	X-Axis Data 0	*/
	uint8_t	DATAX1;	/*	X-Axis Data 1	*/
	uint8_t	DATAY0;	/*	Y-Axis Data 0	*/
	uint8_t	DATAY1;	/*	Y-Axis Data 1	*/
	uint8_t	DATAZ0;	/*	Z-Axis Data 0	*/
	uint8_t	DATAZ1;	/*	Z-Axis Data 1	*/
	uint8_t	FIFO_CTL;	/*	FIFO control	*/
	uint8_t	FIFO_STATUS;	/*	FIFO status	*/
};

enum AdxlBitNum
{
	D0,
	D1,
	D2,
	D3,
	D4,
	D5,
	D6,
	D7
};

enum AdxlStats
{
	ADXL_PENDING,
	ADXL_FAULT,
	ADXL_DBL_TAP,
	ADXL_ACTIVE,
	ADXL_DEV_ID = 0xE5
};

struct AdxlData
{
	uint8_t id;
	uint8_t activity;
	uint8_t int_src;
};

/* ADXL345 I2C address */
#define ADXL_ADR (0x53 << 1)

void adxl_init(void);
void adxl_readData(uint8_t reg, uint8_t *data, uint8_t len);
/* ACTIVITY */
void adxl_activity_init(void);

/* DOUBLE TAP */
void adxl_doubletap_init(void);

/* DEVICE ID */
void adxl_device_id(void);

#endif /*__ADXL_H*/
