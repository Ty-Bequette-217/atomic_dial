#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "lvgl.h"

typedef enum {
    UI_MODE_UNBOUNDED_NO_DETENTS = 0,
    UI_MODE_SOFT_DETENTS,
    UI_MODE_STRONG_DETENTS,
    UI_MODE_COUNT
} ui_mode_t;

// Initializes the screen background, arc, and styles
void ui_init(void);

// Shows the startup welcome screen before normal encoder updates begin
void ui_play_welcome_screen(void);

// Safely updates the position of the ball on the track
void ui_update_arc(uint16_t value);

// Updates the mode and detent labels shown below the value
void ui_set_mode(ui_mode_t mode);

// Updates the centered text shown on the display
void ui_set_center_value(uint16_t value);

#endif // UI_H
