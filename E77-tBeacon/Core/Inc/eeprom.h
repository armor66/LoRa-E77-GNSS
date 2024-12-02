#ifndef EEPROM_H
#define EEPROM_H

/* Includes ------------------------------------------------------------------*/

#include "stm32wlxx.h"

/* Declarations and definitions ----------------------------------------------*/
#define FLASH_PAGE_SIZE		0x00000800U
#define	FLASH_BASE			0x08000000UL

/*!< Get page index from page address */
#define PAGE(__ADDRESS__) (uint32_t)((((__ADDRESS__) - FLASH_BASE) % FLASH_BANK_SIZE) / FLASH_PAGE_SIZE)
///* Functions -----------------------------------------------------------------*/

void flash_lock(void);
uint8_t flash_unlock(void);
void flash_erase_page(uint32_t page);
void flash_write_array(uint32_t pDestination, uint8_t *pSource, uint32_t uLength);
//void write_page(uint32_t start_address, uint8_t data_array[], uint16_t amount);
void read_page(uint32_t start_address, uint8_t data_array[], uint16_t amount);

//typedef enum {
//  PAGE_CLEARED = 0xFFFFFFFF,
//  PAGE_ACTIVE = 0x00000000,
//  PAGE_RECEIVING_DATA = 0x55555555,
//} PageState;
//
//typedef enum {
//  PAGE_0 = 0,
//  PAGE_1 = 1,
//  PAGES_NUM = 2,
//} PageIdx;
//

#endif // #ifndef EEPROM_H
