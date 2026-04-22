#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include "pico/time.h"

// 1: Enable the config file
#define LV_CONF_SKIP 0

// Color depth: 16-bit RGB565 for the GC9A01
#define LV_COLOR_DEPTH 16

// Memory management: Let LVGL use standard C malloc/free
#define LV_MEM_CUSTOM 1

// Tell LVGL how to get the current time in milliseconds
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "pico/time.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (to_ms_since_boot(get_absolute_time()))

// Enable default fonts
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_48 1

#endif /*LV_CONF_H*/
