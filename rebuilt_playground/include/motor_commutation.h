#ifndef MOTOR_COMMUTATION_H
#define MOTOR_COMMUTATION_H

#include <stdint.h>

typedef struct {
    uint16_t uh, ul;
    uint16_t vh, vl;
    uint16_t wh, wl;
} phase_pwm_t;

void motor_commutation_sine(float electrical_angle,
                            float torque,
                            uint16_t pwm_wrap,
                            phase_pwm_t *out);

#endif