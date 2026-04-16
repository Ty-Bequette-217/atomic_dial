#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "board_config.h"
#include "encoder_mt6701.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline void encoder_cs_select(uint pin) {
    gpio_put(pin, 0);
}

static inline void encoder_cs_deselect(uint pin) {
    gpio_put(pin, 1);
}

void encoder_mt6701_init(encoder_mt6701_t *enc) {
    enc->spi = MAG_SPI;
    enc->cs_pin = MAG_CSN_PIN;
    enc->last_raw = 0;
    enc->last_angle_rad = 0.0f;
    enc->velocity_rad_s = 0.0f;
    enc->initialized = 1;

    spi_init(enc->spi, 1000 * 1000);

    gpio_set_function(MAG_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MAG_DO_PIN, GPIO_FUNC_SPI);

    gpio_init(enc->cs_pin);
    gpio_set_dir(enc->cs_pin, GPIO_OUT);
    gpio_put(enc->cs_pin, 1);
}

uint16_t encoder_mt6701_read_raw(encoder_mt6701_t *enc) {
    uint8_t tx[2] = {0x00, 0x00};
    uint8_t rx[2] = {0x00, 0x00};

    encoder_cs_select(enc->cs_pin);
    spi_write_read_blocking(enc->spi, tx, rx, 2);
    encoder_cs_deselect(enc->cs_pin);

    uint16_t raw = ((uint16_t)rx[0] << 8) | rx[1];
    raw &= 0x3FFF;

    enc->last_raw = raw;
    return raw;
}

float encoder_mt6701_read_angle_rad(encoder_mt6701_t *enc) {
    uint16_t raw = encoder_mt6701_read_raw(enc);
    float angle = (2.0f * (float)M_PI * (float)raw) / ENCODER_COUNTS_PER_REV;
    return angle;
}

float encoder_mt6701_read_velocity_rad_s(encoder_mt6701_t *enc, float dt) {
    float angle = encoder_mt6701_read_angle_rad(enc);
    float delta = angle - enc->last_angle_rad;

    if (delta > (float)M_PI) {
        delta -= 2.0f * (float)M_PI;
    } else if (delta < -(float)M_PI) {
        delta += 2.0f * (float)M_PI;
    }

    enc->velocity_rad_s = delta / dt;
    enc->last_angle_rad = angle;

    return enc->velocity_rad_s;
}
