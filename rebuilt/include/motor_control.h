#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include "motor_pwm.h"

typedef struct {
    motor_pwm_t *pwm;
    float electrical_angle_rad;
    float zero_electrical_offset;
    int motor_direction;
    uint8_t pole_pairs;
    bool is_calibrated;
} motor_control_t;

void motor_control_init(motor_control_t *mc, motor_pwm_t *pwm);
void motor_control_update(motor_control_t *mc, float mech_angle_rad, float torque_cmd, float dt);

#endif