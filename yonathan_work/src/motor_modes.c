#include "motor_modes.h"
#include <math.h>
#include <stdint.h>
#include "motor_feedback.h"
#include "ui/ui.h"

void motor_modes_init(ui_mode_t initial_mode) {
    motor_modes_set_mode(initial_mode);
}

void motor_modes_set_mode(ui_mode_t mode) {
    ui_set_mode(mode);
    ui_update_arc(0);
    ui_set_center_value(0);
}

void motor_modes_update(ui_mode_t mode) {
    float smooth_degrees = 0.0f;
    int32_t logical_step = 0;
    
    // Get the high-fidelity angle and the integer state from the physics engine
    motor_feedback_get_angle_degrees(&smooth_degrees);
    motor_feedback_get_logical_position(&logical_step);

    if (mode == UI_MODE_UNBOUNDED_NO_DETENTS || mode == UI_MODE_SOFT_DETENTS) {
        ui_update_arc((uint16_t)smooth_degrees);
        ui_set_center_value((uint16_t)smooth_degrees);
    } 
    else if (mode == UI_MODE_STRONG_DETENTS) {
        ui_update_arc((uint16_t)smooth_degrees);
        // The text snaps to the solid integer state (e.g., 0, 1, 2...)
        ui_set_center_value((uint16_t)logical_step);
    }
    else if (mode == UI_MODE_ON_OFF_SWITCH) {
        // Map 0-140 degrees physical throw to the -70/70 UI markers
        float mapped_degrees = smooth_degrees - 70.0f;
        if (mapped_degrees < 0.0f) mapped_degrees += 360.0f;
        
        ui_update_arc((uint16_t)mapped_degrees);
        ui_set_center_text(logical_step > 0 ? "ON" : "OFF");
    }
}