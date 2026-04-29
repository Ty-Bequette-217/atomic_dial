#ifndef MOTOR_FEEDBACK_H
#define MOTOR_FEEDBACK_H

#include <stdbool.h>
#include "ui/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

bool motor_feedback_init(ui_mode_t initial_mode);
void motor_feedback_set_mode(ui_mode_t mode);
void motor_feedback_update(void);
bool motor_feedback_get_angle_degrees(float *degrees);
bool motor_feedback_get_switch_position(uint8_t *position);
bool motor_feedback_get_switch_display_degrees(float *degrees);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_FEEDBACK_H
