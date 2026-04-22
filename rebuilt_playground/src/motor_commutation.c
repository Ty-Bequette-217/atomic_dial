#include "motor_commutation.h"

#include <math.h>

static float clamp_unit(float value) {
    if (value > 1.0f) {
        return 1.0f;
    }
    if (value < -1.0f) {
        return -1.0f;
    }
    return value;
}

static void phase_to_pwm(float phase_value, uint16_t pwm_wrap,
                         uint16_t *high_side, uint16_t *low_side) {
    float magnitude = fabsf(phase_value);
    uint16_t duty = (uint16_t)(magnitude * (float)pwm_wrap);

    if (phase_value >= 0.0f) {
        *high_side = duty;
        *low_side = 0;
    } else {
        *high_side = 0;
        *low_side = duty;
    }
}

void motor_commutation_sine(float electrical_angle,
                            float torque,
                            uint16_t pwm_wrap,
                            phase_pwm_t *out) {
    const float torque_unit = clamp_unit(torque);
    const float phase_u = sinf(electrical_angle) * torque_unit;
    const float phase_v = sinf(electrical_angle - 2.09439510239f) * torque_unit;
    const float phase_w = sinf(electrical_angle + 2.09439510239f) * torque_unit;

    phase_to_pwm(phase_u, pwm_wrap, &out->uh, &out->ul);
    phase_to_pwm(phase_v, pwm_wrap, &out->vh, &out->vl);
    phase_to_pwm(phase_w, pwm_wrap, &out->wh, &out->wl);
}
