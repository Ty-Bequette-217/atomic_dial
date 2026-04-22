#ifndef AS5600_H
#define AS5600_H
// PORT OF AS5600 Arduino library for rp2350b / pico

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

#define AS5600_DEFAULT_ADDR 0x36

#define AS5600_REG_ZMCO         0x00
#define AS5600_REG_ZPOS_H       0x01
#define AS5600_REG_MPOS_H       0x03
#define AS5600_REG_MANG_H       0x05
#define AS5600_REG_CONF_H       0x07
#define AS5600_REG_STATUS       0x0B
#define AS5600_REG_RAWANGLE_H   0x0C
#define AS5600_REG_ANGLE_H      0x0E
#define AS5600_REG_AGC          0x1A
#define AS5600_REG_MAGNITUDE_H  0x1B
#define AS5600_REG_BURN         0xFF

typedef enum {
    AS5600_POWER_MODE_NOM  = 0x00,
    AS5600_POWER_MODE_LPM1 = 0x01,
    AS5600_POWER_MODE_LPM2 = 0x02,
    AS5600_POWER_MODE_LPM3 = 0x03
} as5600_power_mode_t;

typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
} as5600_t;

bool as5600_init(as5600_t *dev, i2c_inst_t *i2c, uint8_t addr);

bool as5600_get_raw_angle(as5600_t *dev, uint16_t *angle);
bool as5600_get_angle(as5600_t *dev, uint16_t *angle);
bool as5600_get_magnitude(as5600_t *dev, uint16_t *mag);
bool as5600_get_agc(as5600_t *dev, uint8_t *agc);

bool as5600_is_magnet_detected(as5600_t *dev, bool *detected);
bool as5600_is_agc_min_overflow(as5600_t *dev, bool *overflow);
bool as5600_is_agc_max_overflow(as5600_t *dev, bool *overflow);

bool as5600_set_z_position(as5600_t *dev, uint16_t pos);
bool as5600_get_z_position(as5600_t *dev, uint16_t *pos);

bool as5600_set_power_mode(as5600_t *dev, as5600_power_mode_t mode);
bool as5600_get_power_mode(as5600_t *dev, as5600_power_mode_t *mode);

bool as5600_init_default(void);
float as5600_get_angle_radians(void);

#endif
