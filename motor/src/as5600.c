#include "as5600.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "board_config.h"

#define AS5600_RAW_ANGLE_HI_REG 0x0C

static bool as5600_read_registers(uint8_t start_reg, uint8_t *buffer, size_t len) {
    int w = i2c_write_blocking(MAG_ENC_I2C_PORT, AS5600_I2C_ADDR, &start_reg, 1, true);
    if (w < 0) {
        return false;
    }

    int r = i2c_read_blocking(MAG_ENC_I2C_PORT, AS5600_I2C_ADDR, buffer, len, false);
    return r >= 0;
}

bool as5600_init(void) {
    i2c_init(MAG_ENC_I2C_PORT, 400 * 1000);

    gpio_set_function(MAG_ENC_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(MAG_ENC_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MAG_ENC_I2C_SDA);
    gpio_pull_up(MAG_ENC_I2C_SCL);

    uint8_t buf[2];
    return as5600_read_registers(AS5600_RAW_ANGLE_HI_REG, buf, 2);
}

bool as5600_read_raw(uint16_t *raw_angle) {
    if (raw_angle == NULL) {
        return false;
    }

    uint8_t buf[2];
    if (!as5600_read_registers(AS5600_RAW_ANGLE_HI_REG, buf, 2)) {
        return false;
    }

    *raw_angle = (((uint16_t)buf[0] << 8) | buf[1]) & 0x0FFF;
    return true;
}

bool as5600_read_degrees(float *degrees) {
    if (degrees == NULL) {
        return false;
    }

    uint16_t raw_angle;
    if (!as5600_read_raw(&raw_angle)) {
        return false;
    }

    *degrees = (raw_angle * 360.0f) / 4096.0f;
    return true;
}
