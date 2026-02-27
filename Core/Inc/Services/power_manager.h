#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdint.h>

void power_manager_notify_activity(void);
void power_manager_update(void);
void power_manager_sleep(void);
void power_manager_debug_startup_blink(void);

#endif // POWER_MANAGER_H
