#include "motor_task.h"
#include "tmc6300_driver.h"
#include "motor_commutation.h"
#include "as5600.h"
#include <string.h>
#include <math.h>
#include "board_config.h"


void motor_task_init(motor_task_t *mt) {
    memset(mt, 0, sizeof(*mt));

    mt->pole_pairs = 7;          // change if your motor differs
    mt->motor_direction = 1;     // or -1 if direction is reversed
    mt->enabled = true;
    mt->initialized = true;

    tmc6300_init();
}

void motor_task_set_config(motor_task_t *mt, const SmartKnobConfig *config) {
    mt->config = config;

    if (config) {
        mt->current_position = config->position;
        mt->sub_position_unit = config->sub_position_unit;
        mt->current_detent_center =
            (float)config->position * config->position_width_radians;
    }
}

void motor_task_set_enabled(motor_task_t *mt, bool enabled) {
    mt->enabled = enabled;

    if (!enabled) {
        tmc6300_disable_all();
    }
}

void motor_task_calibrate(motor_task_t *mt) {
    mt->zero_electrical_offset = 0.0f;
    mt->calibrated = true;
}

void motor_task_update(motor_task_t *mt) {
    if (!mt->initialized || !mt->enabled || !mt->config) {
        return;
    }

    float mech_angle = as5600_get_angle_radians();
    mt->shaft_angle_rad = mech_angle;

    float elec_angle =
        mech_angle * mt->pole_pairs * mt->motor_direction
        + mt->zero_electrical_offset;
    mt->electrical_angle_rad = elec_angle;

    float torque_cmd = 0.0f;

    // detent logic goes here later, using mt->config

    phase_pwm_t pwm;
    motor_commutation_sine(elec_angle, torque_cmd, MOTOR_PWM_WRAP, &pwm);

    tmc6300_set_pwm(pwm.uh, pwm.ul,
                    pwm.vh, pwm.vl,
                    pwm.wh, pwm.wl);
}