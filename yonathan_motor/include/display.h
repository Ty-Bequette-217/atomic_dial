#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "lvgl.h"

// Initializes the physical SPI display hardware
void lcd_init(void);

// The custom flush callback required by LVGL to push pixels to the screen
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

#endif // DISPLAY_H