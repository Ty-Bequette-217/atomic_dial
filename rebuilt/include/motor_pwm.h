#ifndef MOTOR_PWM_H
#define MOTOR_PWM_H

#include <stdbool.h>

typedef struct {
    float duty_u;
    float duty_v;
    float duty_w;
    bool enabled;
} motor_pwm_t;

void motor_pwm_init(motor_pwm_t *m);
void motor_pwm_enable(motor_pwm_t *m, bool en);
void motor_pwm_set_duty_3phase(motor_pwm_t *m, float du, float dv, float dw);

#endif