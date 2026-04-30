#ifndef MOTOR_FEEDBACK_H
#define MOTOR_FEEDBACK_H

#include <stdbool.h>
#include <stdint.h>
#include "ui/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

bool motor_feedback_init(ui_mode_t initial_mode);
void motor_feedback_set_mode(ui_mode_t mode);
void motor_feedback_update(void);
void motor_feedback_play_click(bool press);

bool motor_feedback_get_angle_degrees(float *degrees);
float motor_feedback_get_velocity(void);
bool motor_feedback_get_logical_position(int32_t *position);
bool motor_feedback_get_bounds(int32_t *min_pos, int32_t *max_pos);

#ifdef __cplusplus
}
#endif

#endif