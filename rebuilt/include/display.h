#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdio.h>
#include <stdint.h>

void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);
void lcd_init();
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

#endif
