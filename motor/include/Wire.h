#ifndef WIRE_COMPAT_H
#define WIRE_COMPAT_H

#include "Arduino.h"

class TwoWire {
public:
    void begin(void) {}
    void beginTransmission(uint8_t address) { (void)address; }
    size_t write(uint8_t value) { (void)value; return 1; }
    uint8_t endTransmission(bool stop = true) { (void)stop; return 0; }
    uint8_t requestFrom(uint8_t address, uint8_t quantity) {
        (void)address;
        (void)quantity;
        return 0;
    }
    int available(void) { return 0; }
    int read(void) { return 0; }
};

static TwoWire Wire;

#endif // WIRE_COMPAT_H
