#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "lvgl.h"
#include "state_machine.h"

#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 240

// Initializes the screen background, arc, and styles
void ui_init(void);

// Safely updates the position of the ball on the track
void ui_update_arc(uint16_t value);

// update track color based on state
void ui_set_state_visual(knob_state_t state);

#endif // UI_H