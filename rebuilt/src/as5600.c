#include "as5600.h"

#include <math.h>

#include "board_config.h"
#include "hardware/gpio.h"

static as5600_t default_as5600;
static bool default_as5600_ready;

static bool as5600_read_u8(as5600_t *dev, uint8_t reg, uint8_t *value) {
    if (i2c_write_blocking(dev->i2c, dev->addr, &reg, 1, true) != 1) {
        return false;
    }
    return i2c_read_blocking(dev->i2c, dev->addr, value, 1, false) == 1;
}

static bool as5600_read_u16_be(as5600_t *dev, uint8_t reg, uint16_t *value) {
    uint8_t buf[2];
    if (i2c_write_blocking(dev->i2c, dev->addr, &reg, 1, true) != 1) {
        return false;
    }
    if (i2c_read_blocking(dev->i2c, dev->addr, buf, 2, false) != 2) {
        return false;
    }
    *value = ((uint16_t)buf[0] << 8) | buf[1];
    return true;
}

static bool as5600_write_u8(as5600_t *dev, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    return i2c_write_blocking(dev->i2c, dev->addr, buf, 2, false) == 2;
}

static bool as5600_write_u16_be(as5600_t *dev, uint8_t reg, uint16_t value) {
    uint8_t buf[3] = {
        reg,
        (uint8_t)((value >> 8) & 0xFF),
        (uint8_t)(value & 0xFF)
    };
    return i2c_write_blocking(dev->i2c, dev->addr, buf, 3, false) == 3;
}

bool as5600_init(as5600_t *dev, i2c_inst_t *i2c, uint8_t addr) {
    if (!dev || !i2c) return false;
    dev->i2c = i2c;
    dev->addr = addr ? addr : AS5600_DEFAULT_ADDR;
    return true;
}

bool as5600_get_raw_angle(as5600_t *dev, uint16_t *angle) {
    uint16_t v;
    if (!as5600_read_u16_be(dev, AS5600_REG_RAWANGLE_H, &v)) return false;
    *angle = v & 0x0FFF;
    return true;
}

bool as5600_get_angle(as5600_t *dev, uint16_t *angle) {
    uint16_t v;
    if (!as5600_read_u16_be(dev, AS5600_REG_ANGLE_H, &v)) return false;
    *angle = v & 0x0FFF;
    return true;
}

bool as5600_get_magnitude(as5600_t *dev, uint16_t *mag) {
    uint16_t v;
    if (!as5600_read_u16_be(dev, AS5600_REG_MAGNITUDE_H, &v)) return false;
    *mag = v & 0x0FFF;
    return true;
}

bool as5600_get_agc(as5600_t *dev, uint8_t *agc) {
    return as5600_read_u8(dev, AS5600_REG_AGC, agc);
}

bool as5600_is_magnet_detected(as5600_t *dev, bool *detected) {
    uint8_t status;
    if (!as5600_read_u8(dev, AS5600_REG_STATUS, &status)) return false;
    *detected = (status & (1u << 5)) != 0;
    return true;
}

bool as5600_is_agc_min_overflow(as5600_t *dev, bool *overflow) {
    uint8_t status;
    if (!as5600_read_u8(dev, AS5600_REG_STATUS, &status)) return false;
    *overflow = (status & (1u << 3)) != 0;
    return true;
}

bool as5600_is_agc_max_overflow(as5600_t *dev, bool *overflow) {
    uint8_t status;
    if (!as5600_read_u8(dev, AS5600_REG_STATUS, &status)) return false;
    *overflow = (status & (1u << 4)) != 0;
    return true;
}

bool as5600_set_z_position(as5600_t *dev, uint16_t pos) {
    return as5600_write_u16_be(dev, AS5600_REG_ZPOS_H, pos & 0x0FFF);
}

bool as5600_get_z_position(as5600_t *dev, uint16_t *pos) {
    uint16_t v;
    if (!as5600_read_u16_be(dev, AS5600_REG_ZPOS_H, &v)) return false;
    *pos = v & 0x0FFF;
    return true;
}

bool as5600_set_power_mode(as5600_t *dev, as5600_power_mode_t mode) {
    uint16_t conf;
    if (!as5600_read_u16_be(dev, AS5600_REG_CONF_H, &conf)) return false;
    conf &= ~(0x3u << 0);
    conf |= ((uint16_t)mode & 0x3u);
    return as5600_write_u16_be(dev, AS5600_REG_CONF_H, conf);
}

bool as5600_get_power_mode(as5600_t *dev, as5600_power_mode_t *mode) {
    uint16_t conf;
    if (!as5600_read_u16_be(dev, AS5600_REG_CONF_H, &conf)) return false;
    *mode = (as5600_power_mode_t)(conf & 0x3u);
    return true;
}

bool as5600_init_default(void) {
    if (default_as5600_ready) {
        return true;
    }

    i2c_init(MAG_ENC_I2C_PORT, 400000);
    gpio_set_function(MAG_ENC_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(MAG_ENC_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MAG_ENC_I2C_SDA);
    gpio_pull_up(MAG_ENC_I2C_SCL);

    default_as5600_ready = as5600_init(&default_as5600, MAG_ENC_I2C_PORT, AS5600_DEFAULT_ADDR);
    return default_as5600_ready;
}

float as5600_get_angle_radians(void) {
    const float two_pi = 6.28318530718f;
    uint16_t angle_counts = 0;

    if (!as5600_init_default()) {
        return 0.0f;
    }
    if (!as5600_get_angle(&default_as5600, &angle_counts)) {
        return 0.0f;
    }

    return ((float)angle_counts * two_pi) / 4096.0f;
}
