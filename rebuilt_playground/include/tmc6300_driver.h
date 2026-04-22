#ifndef TMC6300_DRIVER_H
#define TMC6300_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

void tmc6300_init(void);
void tmc6300_disable_all(void);
bool tmc6300_has_fault(void);

void tmc6300_set_pwm(uint16_t uh, uint16_t ul,
                     uint16_t vh, uint16_t vl,
                     uint16_t wh, uint16_t wl);

#endif