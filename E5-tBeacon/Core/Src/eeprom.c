/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "bit_band.h"
/* Declarations and definitions ----------------------------------------------*/
//#define FLASH_KEY1               0x45670123
//#define FLASH_KEY2               0xCDEF89AB

//static uint32_t pageAddress[PAGES_NUM] = {PAGE_0_ADDRESS, PAGE_1_ADDRESS};
//static uint32_t varIdList[VAR_NUM] = {PARAM_1, PARAM_2};
//
//uint32_t FLASH_Read(uint32_t address);
//PageState EEPROM_ReadPageState(PageIdx idx);
//EepromResult EEPROM_SetPageState(PageIdx idx, PageState state);
//EepromResult EEPROM_ClearPage(PageIdx idx);
//EepromResult EEPROM_Format();
//EepromResult EEPROM_GetActivePageIdx(PageIdx *idx);
//EepromResult EEPROM_Init();

/* Functions -----------------------------------------------------------------*/
static void UTIL_MEM_cpy_8( void *dst, const void *src, uint16_t size )
{
  uint8_t* dst8= (uint8_t *) dst;
  uint8_t* src8= (uint8_t *) src;

  while( size-- )
    {
        *dst8++ = *src8++;
    }
}
static void flash_doubleWord(uint32_t Address, uint64_t Data)
{
  /* Set PG bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);
  /* Program first word */
  *(uint32_t *)Address = (uint32_t)Data;
  /* Barrier to ensure programming is performed in 2 steps, in right order (independently of compiler optimization behavior) */
  __ISB();
  /* Program second word */
  *(uint32_t *)(Address + 4U) = (uint32_t)(Data >> 32U);
}
static void flash_write64(uint32_t address, uint64_t data)
{
  while (*(uint64_t *)address != data)
  {
	  while (FLASH->SR & FLASH_SR_PESD);	//((READ_BIT(FLASH->SR, FLASH_SR_PESD) == (FLASH_SR_PESD)) ? 1UL : 0UL)

//	  FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
	  while (FLASH->SR & FLASH_SR_BSY);
	  SET_BIT(FLASH->CR, FLASH_CR_PG);
	  flash_doubleWord(address, data);
//	  FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
	  while (FLASH->SR & FLASH_SR_BSY);
	  CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
  }
}

void flash_lock(void)
{
	BIT_BAND_PERI(FLASH->CR, FLASH_CR_LOCK) = 1;	//lock flash
}
uint8_t flash_unlock(void)
{
	if(FLASH->CR & FLASH_CR_LOCK)
	  {
	    /* Authorize the FLASH Registers access */
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	    /* verify Flash is unlock */
		if(FLASH->CR & FLASH_CR_LOCK)
		{
			return 0;
		}
	  }
	return 1;
}

void flash_erase_page(uint32_t page)
{
//	  FLASH_EraseInitTypeDef erase_str;
//	  uint32_t page_error;
//	  erase_str.TypeErase = FLASH_TYPEERASE_PAGES;
//	  erase_str.Page = page;
//	  erase_str.NbPages = 1;
//    HAL_FLASHEx_Erase(&erase_str, &page_error);

	while (FLASH->SR & FLASH_SR_BSY);
//	while (FLASH->SR & FLASH_SR_PESD);	//When set, new program or erase operations are not started
//	BIT_BAND_PERI(FLASH->CR, FLASH_CR_EOPIE) = 1;
	MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((page << FLASH_CR_PNB_Pos) | FLASH_CR_PER | FLASH_CR_STRT));
	__NOP();	//HAL_Delay(10);
	BIT_BAND_PERI(FLASH->CR, FLASH_CR_STRT) = 1;
	while (FLASH->SR & FLASH_SR_BSY);
//		while (!(FLASH->SR & FLASH_SR_EOP));	//set by hardware if the end of operation interrupts are enabled(EOPIE = 1)
//		FLASH->SR = FLASH_SR_EOP;
	CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));	//BIT_BAND_PERI(FLASH->CR, FLASH_CR_PER) = 0;	//unset erase mode
}

void flash_write_array(uint32_t pDestination, uint8_t *pSource, uint32_t uLength)
{
	uint8_t *pSrc = pSource;
	uint64_t src_value;

	for (uint32_t i = 0; i < (uLength / sizeof(uint64_t)); i++)
	{
	    UTIL_MEM_cpy_8(&src_value, pSrc, sizeof(uint64_t));

	    /* Avoid writing 0xFFFFFFFFFFFFFFFFLL on erased Flash */
	    if (src_value != UINT64_MAX)
	    {
	      flash_write64(pDestination, src_value);
	    }

	    pDestination += sizeof(uint64_t);
	    pSrc += sizeof(uint64_t);
	}
}

//void write_page(uint32_t start_address, uint8_t data_array[], uint16_t amount)
//{
//	while (FLASH->SR & FLASH_SR_BSY);	//check busy
//
//	if (FLASH->SR & FLASH_SR_PGSERR)	//Check and clear all error programming flags due to a previous programming. If not, PGSERR is set
//	{
//		FLASH->SR = FLASH_SR_PGSERR;	//clear PGSERR
//	}
//
//	if (!(FLASH->CR & FLASH_CR_LOCK))	//if unlock is successful
//	{
////		BIT_BAND_PERI(FLASH->CR, FLASH_CR_EOPIE) = 1;
//		BIT_BAND_PERI(FLASH->CR, FLASH_CR_PG) = 1;	//set page write mode
//
//		uint64_t array[256];
//		for (uint8_t i = 0; i < 8; i++)
//		{
//			array[0] = ((uint64_t)(data_array[i+7]) << 56)
//		                 | ((uint64_t)(data_array[i+6]) << 48)
//		                 | ((uint64_t)(data_array[i+5]) << 40)
//		                 | ((uint64_t)(data_array[i+4]) << 32)
//		                 | ((uint64_t)(data_array[i+3]) << 24)
//		                 | ((uint64_t)(data_array[i+2]) << 16)
//		                 | ((uint64_t)(data_array[i+1]) << 8)
//		                 | ((uint64_t)(data_array[i]));
//		}
//
//		flash_write64(start_address, array[0]);
//
////		for (uint8_t i = 0; i < (amount / 8); i++)
////		{
////			((__IO uint64_t *)start_address)[i] = (uint64_t)data_array[i];	//write
////			while (FLASH->SR & FLASH_SR_BSY);	//check busy
//////			while (!(FLASH->SR & FLASH_SR_EOP));	//wait
//////			FLASH->SR = FLASH_SR_EOP;
////		}
//		while (FLASH->SR & FLASH_SR_BSY);
//		BIT_BAND_PERI(FLASH->CR, FLASH_CR_PG) = 0;	//unset write mode
//	}
//}

void read_page(uint32_t start_address, uint8_t data_array[], uint16_t amount)
{
	for (uint8_t i = 0; i < amount; i++)
	{
		data_array[i] = ((__IO uint8_t *)start_address)[i];
	}
}

//EepromResult EEPROM_WriteData(uint32_t address, uint32_t varId, uint32_t varValue)
//{
//  EepromResult res = EEPROM_OK;
//  HAL_StatusTypeDef flashRes = HAL_OK;
//  uint64_t fullData = ((uint64_t)varValue << 32) | (uint64_t)varId;
//
//  HAL_FLASH_Unlock();
//  flashRes = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, fullData);
//  HAL_FLASH_Lock();
//
//  if (flashRes != HAL_OK)
//  {
//    res = EEPROM_ERROR;
//  }
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_Read(uint32_t varId, uint32_t *varValue)
//{
//  EepromResult res = EEPROM_OK;
//
//  PageIdx activePage = PAGE_0;
//  res = EEPROM_GetActivePageIdx(&activePage);
//
//  if (res != EEPROM_OK)
//  {
//    return res;
//  }
//
//  uint32_t startAddr = pageAddress[activePage] + PAGE_DATA_OFFSET;
//  uint32_t endAddr = pageAddress[activePage] + PAGE_SIZE - PAGE_DATA_SIZE;
//  uint32_t addr = endAddr;
//
//  uint8_t dataFound = 0;
//
//  while (addr >= startAddr)
//  {
//    uint32_t idData = FLASH_Read(addr);
//    if (idData == varId)
//    {
//      dataFound = 1;
//      *varValue = FLASH_Read(addr + 4);
//      break;
//    }
//    else
//    {
//      addr -= PAGE_DATA_SIZE;
//    }
//  }
//
//  if (dataFound == 0)
//  {
//    res = EEPROM_ERROR;
//  }
//
//  return res;
//}
//
///******************************************************************************/
//EepromResult EEPROM_PageTransfer(PageIdx activePage, uint32_t varId, uint32_t varValue)
//{
//  EepromResult res = EEPROM_OK;
//  PageIdx oldPage, newPage;
//
//  if (activePage == PAGE_0)
//  {
//    oldPage = PAGE_0;
//    newPage = PAGE_1;
//  }
//  else
//  {
//    if (activePage == PAGE_1)
//    {
//      oldPage = PAGE_1;
//      newPage = PAGE_0;
//    }
//  }
//
//  res = EEPROM_SetPageState(newPage, PAGE_RECEIVING_DATA);
//
//  if (res != EEPROM_OK)
//  {
//    return res;
//  }
//
//  uint32_t curAddr = pageAddress[newPage] + PAGE_DATA_OFFSET;
//  res = EEPROM_WriteData(curAddr, varId, varValue);
//
//  if (res != EEPROM_OK)
//  {
//    return res;
//  }
//
//  curAddr += PAGE_DATA_SIZE;
//
//  for (uint32_t curVar = 0; curVar < VAR_NUM; curVar++)
//  {
//    if (varIdList[curVar] != varId)
//    {
//      uint32_t curVarValue = 0;
//      res = EEPROM_Read(varIdList[curVar], &curVarValue);
//
//      if (res == EEPROM_OK)
//      {
//        res = EEPROM_WriteData(curAddr, varIdList[curVar], curVarValue);
//
//        if (res != EEPROM_OK)
//        {
//          return res;
//        }
//
//        curAddr += PAGE_DATA_SIZE;
//      }
//    }
//  }
//
//  res = EEPROM_ClearPage(oldPage);
//
//  if (res != EEPROM_OK)
//  {
//    return res;
//  }
//
//  res = EEPROM_SetPageState(newPage, PAGE_ACTIVE);
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_Write(uint32_t varId, uint32_t varValue)
//{
//  EepromResult res = EEPROM_OK;
//
//  uint8_t validId = 0;
//  for (uint32_t curVar = 0; curVar < VAR_NUM; curVar++)
//  {
//    if (varIdList[curVar] == varId)
//    {
//      validId = 1;
//      break;
//    }
//  }
//
//  if (validId == 0)
//  {
//    res = EEPROM_ERROR;
//    return res;
//  }
//
//  PageIdx activePage = PAGE_0;
//  res = EEPROM_GetActivePageIdx(&activePage);
//
//  if (res != EEPROM_OK)
//  {
//    return res;
//  }
//
//  uint32_t startAddr = pageAddress[activePage] + PAGE_DATA_OFFSET;
//  uint32_t endAddr = pageAddress[activePage] + PAGE_SIZE - PAGE_DATA_SIZE;
//  uint32_t addr = startAddr;
//
//  uint8_t freeSpaceFound = 0;
//
//  while (addr <= endAddr)
//  {
//    uint32_t idData = FLASH_Read(addr);
//    if (idData == 0xFFFFFFFF)
//    {
//      freeSpaceFound = 1;
//      break;
//    }
//    else
//    {
//      addr += PAGE_DATA_SIZE;
//    }
//  }
//
//  if (freeSpaceFound == 1)
//  {
//    res = EEPROM_WriteData(addr, varId, varValue);
//  }
//  else
//  {
//    res = EEPROM_PageTransfer(activePage, varId, varValue);
//  }
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_CopyPageData(PageIdx oldPage, PageIdx newPage)
//{
//  EepromResult res = EEPROM_OK;
//
//  uint32_t curAddr = pageAddress[newPage] + PAGE_DATA_OFFSET;
//
//  for (uint32_t curVar = 0; curVar < VAR_NUM; curVar++)
//  {
//    uint32_t curVarValue = 0;
//    res = EEPROM_Read(varIdList[curVar], &curVarValue);
//
//    if (res == EEPROM_OK)
//    {
//      res = EEPROM_WriteData(curAddr, varIdList[curVar], curVarValue);
//
//      if (res != EEPROM_OK)
//      {
//        return res;
//      }
//
//      curAddr += PAGE_DATA_SIZE;
//    }
//  }
//
//  res = EEPROM_SetPageState(newPage, PAGE_ACTIVE);
//
//  if (res != EEPROM_OK)
//  {
//    return res;
//  }
//
//  res = EEPROM_ClearPage(oldPage);
//
//  return res;
//}
//
//
//
///******************************************************************************/
//uint32_t FLASH_Read(uint32_t address)
//{
//  return (*(__IO uint32_t*)address);
//}
//
//
//
///******************************************************************************/
//PageState EEPROM_ReadPageState(PageIdx idx)
//{
//  PageState pageState;
//  pageState = (PageState)FLASH_Read(pageAddress[idx]);
//  return pageState;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_SetPageState(PageIdx idx, PageState state)
//{
//  EepromResult res = EEPROM_OK;
//  HAL_StatusTypeDef flashRes = HAL_OK;
//
//  HAL_FLASH_Unlock();
//  flashRes = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, pageAddress[idx], state);
//  HAL_FLASH_Lock();
//
//  if (flashRes != HAL_OK)
//  {
//    res = EEPROM_ERROR;
//  }
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_ClearPage(PageIdx idx)
//{
//  EepromResult res = EEPROM_OK;
//
//  FLASH_EraseInitTypeDef erase;
//
//  erase.TypeErase = FLASH_TYPEERASE_PAGES;
//  erase.Banks = FLASH_BANK_1;
//  erase.PageAddress = pageAddress[idx];
//  erase.NbPages = 1;
//
//  HAL_StatusTypeDef flashRes = HAL_OK;
//  uint32_t pageError = 0;
//
//  HAL_FLASH_Unlock();
//  flashRes = HAL_FLASHEx_Erase(&erase, &pageError);
//  HAL_FLASH_Lock();
//
//  if (flashRes != HAL_OK)
//  {
//    res = EEPROM_ERROR;
//    return res;
//  }
//
//  res = EEPROM_SetPageState(idx, PAGE_CLEARED);
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_Format()
//{
//  EepromResult res = EEPROM_OK;
//
//  for (uint8_t i = 0; i < PAGES_NUM; i++)
//  {
//    res = EEPROM_ClearPage((PageIdx)i);
//
//    if (res != EEPROM_OK)
//    {
//      return res;
//    }
//  }
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_GetActivePageIdx(PageIdx *idx)
//{
//  EepromResult res = EEPROM_OK;
//  PageState pageStates[PAGES_NUM];
//
//  for (uint8_t i = 0; i < PAGES_NUM; i++)
//  {
//    pageStates[i] = EEPROM_ReadPageState((PageIdx)i);
//  }
//
//  if ((pageStates[PAGE_0] == PAGE_ACTIVE) && (pageStates[PAGE_1] != PAGE_ACTIVE))
//  {
//    *idx = PAGE_0;
//  }
//  else
//  {
//    if ((pageStates[PAGE_1] == PAGE_ACTIVE) && (pageStates[PAGE_0] != PAGE_ACTIVE))
//    {
//      *idx = PAGE_1;
//    }
//    else
//    {
//      res = EEPROM_ERROR;
//    }
//  }
//
//  return res;
//}
//
//
//
///******************************************************************************/
//EepromResult EEPROM_Init()
//{
//  EepromResult res = EEPROM_OK;
//  PageState pageStates[PAGES_NUM];
//
//  for (uint8_t i = 0; i < PAGES_NUM; i++)
//  {
//    pageStates[i] = EEPROM_ReadPageState((PageIdx)i);
//  }
//
//  if (((pageStates[PAGE_0] == PAGE_CLEARED) && (pageStates[PAGE_1] == PAGE_CLEARED)) ||
//      ((pageStates[PAGE_0] == PAGE_ACTIVE) && (pageStates[PAGE_1] == PAGE_ACTIVE)) ||
//      ((pageStates[PAGE_0] == PAGE_RECEIVING_DATA) && (pageStates[PAGE_1] == PAGE_RECEIVING_DATA)))
//  {
//    res = EEPROM_Format();
//
//    if (res != EEPROM_OK)
//    {
//      return res;
//    }
//
//    res = EEPROM_SetPageState(PAGE_0, PAGE_ACTIVE);
//  }
//
//  if ((pageStates[PAGE_0] == PAGE_RECEIVING_DATA) && (pageStates[PAGE_1] == PAGE_CLEARED))
//  {
//    res = EEPROM_SetPageState(PAGE_0, PAGE_ACTIVE);
//  }
//
//  if ((pageStates[PAGE_0] == PAGE_CLEARED) && (pageStates[PAGE_1] == PAGE_RECEIVING_DATA))
//  {
//    res = EEPROM_SetPageState(PAGE_1, PAGE_ACTIVE);
//  }
//
//  if ((pageStates[PAGE_0] == PAGE_RECEIVING_DATA) && (pageStates[PAGE_1] == PAGE_ACTIVE))
//  {
//    res = EEPROM_CopyPageData(PAGE_1, PAGE_0);
//  }
//
//  if ((pageStates[PAGE_0] == PAGE_ACTIVE) && (pageStates[PAGE_1] == PAGE_RECEIVING_DATA))
//  {
//    res = EEPROM_CopyPageData(PAGE_0, PAGE_1);
//  }
//
//  return res;
//}

