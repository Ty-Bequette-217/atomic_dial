#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "lvgl.h"

#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 240

// Initializes the screen background, arc, and styles
void ui_init(void);

// Safely updates the position of the ball on the track
void ui_update_arc(uint16_t value);

#endif // UI_H