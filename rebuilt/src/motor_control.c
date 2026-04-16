#include <math.h>

#include "board_config.h"
#include "motor_control.h"
#include "motor_pwm.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float wrap_2pi(float x) {
    while (x >= 2.0f * (float)M_PI) x -= 2.0f * (float)M_PI;
    while (x < 0.0f) x += 2.0f * (float)M_PI;
    return x;
}

void motor_control_init(motor_control_t *mc, motor_pwm_t *pwm) {
    mc->pwm = pwm;
    mc->electrical_angle_rad = 0.0f;
    mc->zero_electrical_offset = 0.0f;
    mc->motor_direction = MOTOR_DEFAULT_DIRECTION;
    mc->pole_pairs = MOTOR_DEFAULT_POLE_PAIRS;
    mc->is_calibrated = false;
}

void motor_control_update(motor_control_t *mc, float mech_angle_rad, float torque_cmd, float dt) {
    (void)dt;

    mc->electrical_angle_rad = wrap_2pi(
        ((float)mc->motor_direction * mech_angle_rad * (float)mc->pole_pairs) +
        mc->zero_electrical_offset
    );

    float amp = 0.20f * torque_cmd;
    if (amp > 0.45f) amp = 0.45f;
    if (amp < -0.45f) amp = -0.45f;

    float a = mc->electrical_angle_rad;

    float du = 0.5f + amp * sinf(a);
    float dv = 0.5f + amp * sinf(a - 2.0f * (float)M_PI / 3.0f);
    float dw = 0.5f + amp * sinf(a + 2.0f * (float)M_PI / 3.0f);

    motor_pwm_set_duty_3phase(mc->pwm, du, dv, dw);
}