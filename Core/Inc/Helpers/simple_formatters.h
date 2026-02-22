#ifndef SIMPLE_FORMATTERS_H
#define SIMPLE_FORMATTERS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Format time as "MM:SS"
 * @param buf Output buffer
 * @param bufsize Buffer size (must be >= 6)
 * @param minutes 0..99
 * @param seconds 0..59
 */
void simple_time_format(char* buf, size_t bufsize, uint8_t minutes, uint8_t seconds);

/**
 * @brief Convert unsigned integer to string with optional prefix
 * @param buf Output buffer
 * @param bufsize Buffer size
 * @param value Unsigned value
 * @param prefix Optional prefix (can be NULL)
 */
void simple_utoa(char* buf, size_t bufsize, unsigned value, const char* prefix);

#endif // SIMPLE_FORMATTERS_H