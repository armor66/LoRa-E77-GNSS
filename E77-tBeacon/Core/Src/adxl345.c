#include "adxl345.h"
#include "bit_band.h"

//__inline:
//static void adxl_delay(uint32_t ms);
static uint8_t adxl_timeout(uint32_t start, uint32_t ms);
static void adxl_writeData(uint8_t reg, uint8_t data);

//void adxl_readData(uint8_t reg, uint8_t *data, uint8_t len);

void i2c_init(void)
{
	/* I2C1 GPIO Configuration */
	//PB6     ------> I2C1_SCL
	GPIOB->MODER   &=~GPIO_MODER_MODE6;
	GPIOB->MODER   |= GPIO_MODER_MODE6_1;		//10: Alternate function mode
	GPIOB->OTYPER  |= GPIO_OTYPER_OT6;			//1: Output open-drain 1 << 6
	GPIOB->OSPEEDR &=~GPIO_OSPEEDR_OSPEED6;		//00: Low speed
	GPIOB->PUPDR   |= GPIO_PUPDR_PUPD6_0;		//01: Pull-up
	GPIOB->AFR[0]  &=~GPIO_AFRL_AFSEL6;
	GPIOB->AFR[0]  |= 4<<GPIO_AFRL_AFSEL6_Pos;
	//PB7     ------> I2C1_SDA
	GPIOB->MODER   &=~GPIO_MODER_MODE7;
	GPIOB->MODER   |= GPIO_MODER_MODE7_1;		//10: Alternate function mode
	GPIOB->OTYPER  |= GPIO_OTYPER_OT7;			//1: Output open-drain 1 << 7
	GPIOB->OSPEEDR &=~GPIO_OSPEEDR_OSPEED7;		//00: Low speed
	GPIOB->PUPDR   |= GPIO_PUPDR_PUPD7_0;		//01: Pull-up
	GPIOB->AFR[0]  &=~GPIO_AFRL_AFSEL7;
	GPIOB->AFR[0]  |= 4<<GPIO_AFRL_AFSEL7_Pos;

    /* I2C1 clock enable */
	RCC->CCIPR &= ~RCC_CCIPR_I2C1SEL;			//00: PCLK selected
//	i2c_clock_enable();
    LL_APB1_GRP1_EnableClock((0x1UL << (21U)));
//	  __IO uint32_t tmpreg;
//	  SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN);
//	  /* Delay after an RCC peripheral clock enabling */
//	  tmpreg = READ_BIT(RCC->APB1ENR1, 0x1UL << (21U));
//	  (void)tmpreg;

    I2C1->TIMINGR = 0x20303E5D;

    I2C1->CR1 |= I2C_CR1_PE;	//enable i2c1

//    i2c_clock_disable();
}

void i2c_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN) = 0;
}

void i2c_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN) = 1;
//	delay_cyc(10);
}

struct AdxlCommands AdxlReg = {0x00, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
	0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
	0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

//void adxl_init(void)
//{
//	/*Enable I2C*/
//	i2c_init();
//
//	/*Read the DEVID, this should return 0xE5*/
////	adxl_device_id();
////	adxl_readData(AdxlReg.DEVID, (uint8_t*)&main_flags.adxl_device_id, 1);
//
//	/*Reset all bits*/
////	adxl_writeData(uint8_t reg, uint8_t data);(POWER_CTL_R, RESET);
//
//	/*Configure power control measure bit*/
////	adxl_writeData(uint8_t reg, uint8_t data);(POWER_CTL_R, SET_MEASURE_B);
//}

/* DEVICE ID */
void adxl_device_id(void)
{
HAL_I2C_Master_Transmit(&hi2c1, ADXL_ADR, &AdxlReg.DEVID, 1, 100);
HAL_I2C_Master_Receive(&hi2c1, ADXL_ADR, (uint8_t*)&main_flags.adxl_device_id, 1, 100);
}

uint8_t i2c_tx[2];

/* ACTIVITY */
void adxl_activity_init(void)
{
	/* Activity Axes */
	adxl_writeData(AdxlReg.ACT_INACT_CTL, 0xF0);	//((1 << D7) | (1 << D6) | (1 << D5) | (1 << D4))
	/* Threshold Activity */
	adxl_writeData(AdxlReg.THRESH_ACT, 60);
	/* Int Map */
	adxl_writeData(AdxlReg.INT_MAP, 0x30);			//~(1 << D4) 0 for INT1
	/* Interrupt Enable */
//	adxl_writeData(AdxlReg.INT_ENABLE, 0x10);		//(1 << D4)
	/* Measure */
	adxl_writeData(AdxlReg.POWER_CTL, 0x08);		//(1 << D3)
}

/* DOUBLE TAP */
void adxl_doubletap_init(void)
{
	/* Tap Axes */
	adxl_writeData(AdxlReg.TAP_AXES, 0x01);			//(1 << D0)	//Z only ((1 << D2) | (1 << D1) | )
	/* Threshold Tap */
	adxl_writeData(AdxlReg.THRESH_TAP, 0x7F);		//60
	/* Tap Duration */
	adxl_writeData(AdxlReg.DUR, 40);
	/* Tap Latency */
	adxl_writeData(AdxlReg.Latent, 0x7F);			//80
	/* Tap Window */
	adxl_writeData(AdxlReg.Window, 0xFF);			//200
	/* Interrupt Enable */
//	adxl_writeData(AdxlReg.INT_ENABLE, 0x20);		//(1 << D5)
	/* Interrupt Map */
//	adxl_writeData(AdxlReg.INT_MAP, 0x20);			//~(1 << D5); //0 for INT1
	/* Measure */
//	adxl_writeData(AdxlReg.POWER_CTL, 0x08);		//(1 << D3)
}

void adxl_init(void)	//DOUBLE TAP & ACTIVITY
{
	/* Tap Axes */
	adxl_writeData(AdxlReg.TAP_AXES, 0x04);			//Z only (1 << D0)	//except Z ((1 << D2) | (1 << D1) | )
	/* Threshold Tap */
	adxl_writeData(AdxlReg.THRESH_TAP, 0x7F);		//60
	/* Tap Duration */
	adxl_writeData(AdxlReg.DUR, 40);
	/* Tap Latency */
	adxl_writeData(AdxlReg.Latent, 0x7F);			//80
	/* Tap Window */
	adxl_writeData(AdxlReg.Window, 0xFF);			//200

	/* Activity Axes */
	adxl_writeData(AdxlReg.ACT_INACT_CTL, 0xB0);	//except  ((1 << D7) | (1 << D6) | (1 << D5) | (1 << D4))
	/* Threshold Activity */
	adxl_writeData(AdxlReg.THRESH_ACT, 60);

	/* Int Map */
	adxl_writeData(AdxlReg.INT_MAP, 0x30);			//for INT2
//	/* Interrupt Enable */
	adxl_writeData(AdxlReg.INT_ENABLE, 0x30);		//(1 << D4)

	/* Measure */
	adxl_writeData(AdxlReg.POWER_CTL, 0x08);		//(1 << D3)
}
//==============================================================================
/*static __inline functions*/
//==============================================================================
//__inline static void adxl_delay(uint32_t ms) {
///*  HAL_Delay(time);*/
//  uint32_t tickstart = HAL_GetTick();
//  uint32_t wait = ms;
//
//  /* Add a freq to guarantee minimum wait */
//  if (wait < HAL_MAX_DELAY) wait += (uint32_t)(uwTickFreq);
//
//  while ((HAL_GetTick() - tickstart) < wait){}
//}

__inline static uint8_t adxl_timeout(uint32_t start, uint32_t ms)
{
	if((HAL_GetTick() - start) > ms) {
		I2C1->CR1 &= ~I2C_CR1_PE;	//disable i2c1
		main_flags.adxl_fault = 1;
		main_flags.adxl_status = ADXL_FAULT;
		return 1;
	}else return 0;
}

__inline static void adxl_writeData(uint8_t reg, uint8_t data)
{
	uint8_t txdata[2] = {reg, data};
	uint8_t count = sizeof(txdata);
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(adxl_timeout(tickstart, timeout_ms)) break;
    }
//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (count << I2C_CR2_NBYTES_Pos) | ADXL_ADR;
	I2C1->CR2 |= I2C_CR2_START;

	for(uint8_t i = 0; i < count; i++)
	{
		while(!(I2C1->ISR & I2C_ISR_TXIS)) {
			if(adxl_timeout(tickstart, timeout_ms)) break;
	    }
		/* Write data to TXDR */
		I2C1->TXDR = txdata[i];
		/* Increment Buffer pointer */
	}
/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
	/* Wait until STOPF flag is set */
    while(!(I2C1->ISR & I2C_ISR_STOPF)) {
    	if(adxl_timeout(tickstart, timeout_ms)) break;
    }
    /* Clear STOP Flag */
    I2C1->ICR |= I2C_ICR_STOPCF;
}

void adxl_readData(uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t timeout_ms = 10;
	uint32_t tickstart = HAL_GetTick();

	while (I2C1->ISR & I2C_ISR_BUSY) {
		if(adxl_timeout(tickstart, timeout_ms)) break;
	}

//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (1 << I2C_CR2_NBYTES_Pos) | ADXL_ADR;
	I2C1->CR2 |= I2C_CR2_START;

	while(!(I2C1->ISR & I2C_ISR_TXIS)) {
		if(adxl_timeout(tickstart, timeout_ms)) break;
	}
	/* Write data to TXDR */
	I2C1->TXDR = reg;
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(adxl_timeout(tickstart, timeout_ms)) break;
	}
	I2C1->ICR |= I2C_ICR_STOPCF;

//	HAL_I2C_Master_Receive(_adxl_i2c_port, BNO055_I2C_ADDR << 1, data, len, 100);
//	I2C1->CR2 &= ~0x3FFFFFF;
	I2C1->CR2 = I2C_CR2_AUTOEND | (len << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN | ADXL_ADR;
	I2C1->CR2 |= I2C_CR2_START;

	for(uint8_t i = 0; i < len; i++)
	{
		tickstart = HAL_GetTick();
		while(!(I2C1->ISR & I2C_ISR_RXNE)) {
			if(adxl_timeout(tickstart, timeout_ms)) break;
		}
		data[i] = I2C1->RXDR;
	}
/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
	while(!(I2C1->ISR & I2C_ISR_STOPF)) {
		if(adxl_timeout(tickstart, timeout_ms)) break;
	}
	/* Clear STOP Flag */
	I2C1->ICR |= I2C_ICR_STOPCF;
}
