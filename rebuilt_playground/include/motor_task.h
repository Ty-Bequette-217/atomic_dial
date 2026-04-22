#ifndef MOTOR_TASK_H
#define MOTOR_TASK_H

#include <stdbool.h>
#include <stdint.h>
#include "state_machine.h"

typedef struct {
    const SmartKnobConfig *config;

    float zero_electrical_offset;
    uint8_t pole_pairs;
    int8_t motor_direction;

    int32_t current_position;
    float shaft_angle_rad;
    float electrical_angle_rad;
    float sub_position_unit;

    float current_detent_center;
    float idle_check_velocity_ewma;

    bool initialized;
    bool calibrated;
    bool enabled;
} motor_task_t;

void motor_task_init(motor_task_t *mt);
void motor_task_calibrate(motor_task_t *mt);
void motor_task_update(motor_task_t *mt);
void motor_task_set_config(motor_task_t *mt, const SmartKnobConfig *config);
void motor_task_set_enabled(motor_task_t *mt, bool enabled);

#endif