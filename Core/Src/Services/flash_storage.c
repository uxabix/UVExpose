#include "Services/flash_storage.h"
#include "stm32f1xx_hal.h"

#include <string.h>

bool FlashStorage_Read(uint32_t address, void* dst, uint16_t len)
{
    if (dst == NULL || len == 0u) {
        return false;
    }

    memcpy(dst, (const void*)address, len);
    return true;
}

bool FlashStorage_ErasePage(uint32_t page_address)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0u;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = page_address;
    erase.NbPages = 1u;

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &page_error);
    HAL_FLASH_Lock();

    return (status == HAL_OK) && (page_error == 0xFFFFFFFFu);
}

bool FlashStorage_ProgramHalfWord(uint32_t address, uint16_t value)
{
    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, value);
    HAL_FLASH_Lock();

    return (status == HAL_OK);
}

bool FlashStorage_ProgramWord(uint32_t address, uint32_t value)
{
    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, value);
    HAL_FLASH_Lock();

    return (status == HAL_OK);
}
