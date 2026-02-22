#include "Services/settings_service.h"
#include "config.h"
#include "stm32f1xx_hal.h"

#include <stdbool.h>
#include <string.h>

settings_t g_settings;

uint32_t Settings_CalcCRC(const settings_t *cfg)
{
    uint32_t crc = 0;
    const uint8_t *data = (const uint8_t*)cfg;
    for (size_t i = 0; i < sizeof(settings_t) - sizeof(uint32_t); ++i)
        crc += data[i];
    return crc;
}

void Settings_Save(const settings_t *cfg)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    uint32_t page_error;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = SETTINGS_FLASH_ADDR;
    erase.NbPages = 1;

    HAL_FLASHEx_Erase(&erase, &page_error);

    settings_t temp = *cfg;
    temp.crc = Settings_CalcCRC(cfg);

    uint32_t *data = (uint32_t*)&temp;
    for(uint32_t i = 0; i < sizeof(settings_t)/4; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                          SETTINGS_FLASH_ADDR + i*4,
                          data[i]);
    }

    HAL_FLASH_Lock();
}

int Settings_Load(settings_t *cfg)
{
    memcpy(cfg, (void*)SETTINGS_FLASH_ADDR, sizeof(settings_t));
    uint32_t crc = Settings_CalcCRC(cfg);
    if (cfg->crc != crc)
        return 0;
    return 1;
}

void Settings_Init(void)
{
    if (!Settings_Load(&g_settings))
    {
        // Settings are not valid, load defaults
        g_settings.burn_in_protection = true;
        g_settings.open_lid_protection = true;
        g_settings.beep_count = 1;
        g_settings.beep_duration = 300;
        g_settings.beep_period = 200;
        g_settings.sleep_mode = 1; // 2 minutes
        g_settings.lid_open_threshold_mv = LID_HALL_OPEN_THRESHOLD_MV;
        g_settings.lid_close_threshold_mv = LID_HALL_CLOSE_THRESHOLD_MV;

        Settings_Save(&g_settings);
    }
}
