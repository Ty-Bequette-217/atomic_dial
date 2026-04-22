#ifndef AS5600_H
#define AS5600_H

#include <stdbool.h>
#include <stdint.h>

#define AS5600_I2C_ADDR 0x36

#ifdef __cplusplus
extern "C" {
#endif

bool as5600_init(void);
bool as5600_read_raw(uint16_t *raw_angle);
bool as5600_read_degrees(float *degrees);

#ifdef __cplusplus
}
#endif

#endif
