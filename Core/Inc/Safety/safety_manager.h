#ifndef INC_SAFETY_MANAGER_H_
#define INC_SAFETY_MANAGER_H_

#include <stdint.h>

void Safety_Init(void);
void Safety_Process(void);
uint8_t Safety_IsLidOpen(void);
uint8_t Safety_CanSleep(void); // Returns 1 if STOP mode is safe

#endif /* INC_SAFETY_MANAGER_H_ */
