#ifndef IIC_STM32_H_
#define IIC_STM32_H_

#include "i2c.h"

inline void iic_delay(uint32_t ms) {
//  HAL_Delay(time);
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = ms;

  /* Add a freq to guarantee minimum wait */
  if (wait < HAL_MAX_DELAY) wait += (uint32_t)(uwTickFreq);

  while ((HAL_GetTick() - tickstart) < wait){}
}

static inline uint8_t iic_timeout(uint32_t start, uint32_t ms)
{
	if((HAL_GetTick() - start) > ms)
	{
		return 1;
	}else return 0;
}

int8_t iic_writeData(uint8_t addr, uint8_t reg, uint8_t data)
{
	uint8_t txdata[2] = {reg, data};
	uint8_t count = sizeof(txdata);
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
    }
//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (count << I2C_CR2_NBYTES_Pos) | (addr << 1);
	I2C1->CR2 |= I2C_CR2_START;

	for(uint8_t i = 0; i < count; i++)
	{
		while(!(I2C1->ISR & I2C_ISR_TXIS)) {
			if(iic_timeout(tickstart, timeout_ms)) return 1;
	    }
		/* Write data to TXDR */
		I2C1->TXDR = txdata[i];
		/* Increment Buffer pointer */
	}
/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
	/* Wait until STOPF flag is set */
    while(!(I2C1->ISR & I2C_ISR_STOPF)) {
    	if(iic_timeout(tickstart, timeout_ms)) return 1;
    }
    /* Clear STOP Flag */
    I2C1->ICR |= I2C_ICR_STOPCF;
    return 0;
}

int8_t iic_readData(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
	}

//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (1 << I2C_CR2_NBYTES_Pos) | (addr << 1);
	I2C1->CR2 |= I2C_CR2_START;

	while(!(I2C1->ISR & I2C_ISR_TXIS)) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
	}
	/* Write data to TXDR */
	I2C1->TXDR = reg;
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
	}
	I2C1->ICR |= I2C_ICR_STOPCF;

//	HAL_I2C_Master_Receive(_iic_i2c_port, BNO055_I2C_ADDR << 1, data, len, 100);
//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (len << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN | (addr << 1);
	I2C1->CR2 |= I2C_CR2_START;

	for(uint8_t i = 0; i < len; i++)
	{
		tickstart = HAL_GetTick();
		while(!(I2C1->ISR & I2C_ISR_RXNE)) {
			if(iic_timeout(tickstart, timeout_ms)) return 1;
		}
		data[i] = I2C1->RXDR;
	}
/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
	}
	/* Clear STOP Flag */
	I2C1->ICR |= I2C_ICR_STOPCF;
	return 0;
}

int8_t iic_check(uint8_t addr)
{
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
	}

	I2C1->CR2 = I2C_CR2_AUTOEND | (0 << I2C_CR2_NBYTES_Pos) | (addr << 1);
	I2C1->CR2 |= I2C_CR2_START;

/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(iic_timeout(tickstart, timeout_ms)) return 1;
	}

	if (I2C1->ISR & I2C_ISR_NACKF)	/* if the NACKF flag has been set */
	{
		I2C1->ICR |= I2C_ICR_NACKCF;		/* Clear NACK Flag */
		I2C1->ICR |= I2C_ICR_STOPCF;		/* Clear STOP Flag */
		//trials--;
		return 1;
	}
	else						/* if the NACKF flag has not been set */
	{
		I2C1->ICR |= I2C_ICR_STOPCF;		/* Clear STOP Flag */
		return 0;
	}
}
#endif  // IIC_STM32_H_
