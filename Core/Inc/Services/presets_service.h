#ifndef INC_SERVICES_PRESETS_SERVICE_H_
#define INC_SERVICES_PRESETS_SERVICE_H_

#include <stdint.h>

typedef struct {
    uint8_t minutes;      // 0..99
    uint8_t seconds;      // 0..59
    uint8_t buzzer_mode;  // 0=silent, 1=single, 2=multiple
    uint8_t until_off;    // 0/1
} preset_t;

typedef enum {
    PRESETS_STATUS_OK = 0,
    PRESETS_STATUS_DUPLICATE,
    PRESETS_STATUS_FULL,
    PRESETS_STATUS_ERROR,
    PRESETS_STATUS_INVALID_ARG
} presets_status_t;

void Presets_Init(void);
uint8_t Presets_Count(void);
presets_status_t Presets_Get(uint8_t index, preset_t* out_preset);
presets_status_t Presets_Add(const preset_t* preset);
presets_status_t Presets_Delete(uint8_t index);

#endif /* INC_SERVICES_PRESETS_SERVICE_H_ */
