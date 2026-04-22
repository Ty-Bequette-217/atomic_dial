#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "board_config.h"

// Function declarations
void init_motor_control(void);
void enable_motor(void);
void disable_motor(void);
void motor_fault(uint gpio, uint32_t events);

#endif