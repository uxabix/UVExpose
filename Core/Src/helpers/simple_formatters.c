#include "Helpers/simple_formatters.h"

void simple_time_format(char* buf, size_t bufsize, uint8_t minutes, uint8_t seconds)
{
    if (!buf || bufsize < 6) return;

    buf[0] = '0' + (minutes / 10);
    buf[1] = '0' + (minutes % 10);
    buf[2] = ':';
    buf[3] = '0' + (seconds / 10);
    buf[4] = '0' + (seconds % 10);
    buf[5] = '\0';
}

void simple_utoa(char* buf, size_t bufsize, unsigned value, const char* prefix)
{
    if (!buf || bufsize == 0) return;

    size_t i = 0;

    // Copy prefix
    if (prefix) {
        while (prefix[i] && i < bufsize - 1) {
            buf[i] = prefix[i];
            i++;
        }
    }

    // Convert number to reversed string
    char tmp[10];
    int len = 0;

    if (value == 0) {
        tmp[len++] = '0';
    } else {
        while (value && len < (int)sizeof(tmp)) {
            tmp[len++] = '0' + (value % 10);
            value /= 10;
        }
    }

    // Reverse into output buffer
    for (int j = len - 1; j >= 0 && i < bufsize - 1; --j) {
        buf[i++] = tmp[j];
    }

    buf[i] = '\0';
}