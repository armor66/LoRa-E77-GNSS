#ifndef BNO055_STM32_H_
#define BNO055_STM32_H_

#ifdef __cplusplus
  extern "C" {
#endif

#include "main.h"
#include "i2c.h"
#include "bno055.h"

//I2C_HandleTypeDef *_bno055_i2c_port;
//
//void bno055_assignI2C(I2C_HandleTypeDef *hi2c_device) {
//  _bno055_i2c_port = hi2c_device;
//}

inline void bno055_delay(uint32_t ms) {
//  HAL_Delay(time);
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = ms;

  /* Add a freq to guarantee minimum wait */
  if (wait < HAL_MAX_DELAY) wait += (uint32_t)(uwTickFreq);

  while ((HAL_GetTick() - tickstart) < wait){}
}

static inline uint8_t bno055_timeout(uint32_t start, uint32_t ms)
{
	if((HAL_GetTick() - start) > ms) {
		main_flags.compass_fault = 1;
		return 1;
	}else return 0;
}

void bno055_writeData(uint8_t reg, uint8_t data)
{
	uint8_t txdata[2] = {reg, data};
	uint8_t count = sizeof(txdata);
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(bno055_timeout(tickstart, timeout_ms)) break;
    }
//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (count << I2C_CR2_NBYTES_Pos) | (BNO055_I2C_ADDR << 1);
	I2C1->CR2 |= I2C_CR2_START;

	for(uint8_t i = 0; i < count; i++)
	{
		while(!(I2C1->ISR & I2C_ISR_TXIS)) {
			if(bno055_timeout(tickstart, timeout_ms)) break;
	    }
		/* Write data to TXDR */
		I2C1->TXDR = txdata[i];
		/* Increment Buffer pointer */
	}
/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
	/* Wait until STOPF flag is set */
    while(!(I2C1->ISR & I2C_ISR_STOPF)) {
    	if(bno055_timeout(tickstart, timeout_ms)) break;
    }
    /* Clear STOP Flag */
    I2C1->ICR |= I2C_ICR_STOPCF;
}

void bno055_readData(uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(bno055_timeout(tickstart, timeout_ms)) break;
	}

//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (1 << I2C_CR2_NBYTES_Pos) | (BNO055_I2C_ADDR << 1);
	I2C1->CR2 |= I2C_CR2_START;

	while(!(I2C1->ISR & I2C_ISR_TXIS)) {
		if(bno055_timeout(tickstart, timeout_ms)) break;
	}
	/* Write data to TXDR */
	I2C1->TXDR = reg;
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(bno055_timeout(tickstart, timeout_ms)) break;
	}
	I2C1->ICR |= I2C_ICR_STOPCF;

//	HAL_I2C_Master_Receive(_bno055_i2c_port, BNO055_I2C_ADDR << 1, data, len, 100);
//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (len << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN | (BNO055_I2C_ADDR << 1);
	I2C1->CR2 |= I2C_CR2_START;

	for(uint8_t i = 0; i < len; i++)
	{
		tickstart = HAL_GetTick();
		while(!(I2C1->ISR & I2C_ISR_RXNE)) {
			if(bno055_timeout(tickstart, timeout_ms)) break;
		}
		data[i] = I2C1->RXDR;
	}
/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(bno055_timeout(tickstart, timeout_ms)) break;
	}
	/* Clear STOP Flag */
	I2C1->ICR |= I2C_ICR_STOPCF;
}

#ifdef __cplusplus
  }
#endif

#endif  // BNO055_STM32_H_
