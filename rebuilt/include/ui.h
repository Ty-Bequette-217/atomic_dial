#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "lvgl.h"

// Initializes the screen background, arc, and styles
void ui_init(void);

// Safely updates the position of the ball on the track
void ui_update_arc(uint16_t value);

#endif // UI_H