#ifndef INC_SERVICES_FLASH_STORAGE_H_
#define INC_SERVICES_FLASH_STORAGE_H_

#include <stdbool.h>
#include <stdint.h>

bool FlashStorage_Read(uint32_t address, void* dst, uint16_t len);
bool FlashStorage_ErasePage(uint32_t page_address);
bool FlashStorage_ProgramHalfWord(uint32_t address, uint16_t value);
bool FlashStorage_ProgramWord(uint32_t address, uint32_t value);

#endif /* INC_SERVICES_FLASH_STORAGE_H_ */
