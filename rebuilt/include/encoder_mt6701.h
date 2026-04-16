#ifndef ENCODER_MT6701_H
#define ENCODER_MT6701_H

#include <stdint.h>
#include "hardware/spi.h"

typedef struct {
    spi_inst_t *spi;
    uint cs_pin;
    uint16_t last_raw;
    float last_angle_rad;
    float velocity_rad_s;
    int initialized;
} encoder_mt6701_t;

void encoder_mt6701_init(encoder_mt6701_t *enc);
uint16_t encoder_mt6701_read_raw(encoder_mt6701_t *enc);
float encoder_mt6701_read_angle_rad(encoder_mt6701_t *enc);
float encoder_mt6701_read_velocity_rad_s(encoder_mt6701_t *enc, float dt);

#endif