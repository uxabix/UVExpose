#include "Services/settings_service.h"
#include "Services/flash_storage.h"
#include "config.h"

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
    if (!FlashStorage_ErasePage(SETTINGS_FLASH_ADDR)) {
        return;
    }

    settings_t temp = *cfg;
    temp.crc = Settings_CalcCRC(cfg);

    uint32_t *data = (uint32_t*)&temp;
    for(uint32_t i = 0; i < sizeof(settings_t)/4; i++)
    {
        if (!FlashStorage_ProgramWord(SETTINGS_FLASH_ADDR + i*4, data[i])) {
            return;
        }
    }
}

int Settings_Load(settings_t *cfg)
{
    if (!FlashStorage_Read(SETTINGS_FLASH_ADDR, cfg, sizeof(settings_t))) {
        return 0;
    }
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
        g_settings.burn_in_protection = SETTINGS_DEFAULT_BURN_IN_PROTECTION;
        g_settings.open_lid_protection = SETTINGS_DEFAULT_OPEN_LID_PROTECTION;
        g_settings.beep_count = SETTINGS_DEFAULT_BEEP_COUNT;
        g_settings.beep_duration = SETTINGS_DEFAULT_BEEP_DURATION_MS;
        g_settings.beep_period = SETTINGS_DEFAULT_BEEP_PERIOD_MS;
        g_settings.sleep_mode = SETTINGS_DEFAULT_SLEEP_MODE;
        g_settings.lid_open_threshold_mv = SETTINGS_DEFAULT_LID_OPEN_THRESHOLD_MV;
        g_settings.lid_close_threshold_mv = SETTINGS_DEFAULT_LID_CLOSE_THRESHOLD_MV;

        Settings_Save(&g_settings);
    }
}
