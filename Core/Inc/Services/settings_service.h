#ifndef SETTINGS_SERVICE_H
#define SETTINGS_SERVICE_H

#include <stdint.h>

typedef struct {
    uint8_t burn_in_protection;
    uint8_t open_lid_protection;
    uint8_t beep_count;
    uint16_t beep_duration;
    uint16_t beep_period;
    uint8_t sleep_mode;
    uint16_t lid_open_threshold_mv;
    uint16_t lid_close_threshold_mv;
    uint32_t crc;
} settings_t;

extern settings_t g_settings;

uint32_t Settings_CalcCRC(const settings_t *cfg);
void Settings_Save(const settings_t *cfg);
int Settings_Load(settings_t *cfg);
void Settings_Init(void);

#endif // SETTINGS_SERVICE_H
