#ifndef SPI_COMPAT_H
#define SPI_COMPAT_H

// Only let the C++ compiler read the classes!
#ifdef __cplusplus

#include "Arduino.h"

class SPISettings {
public:
    SPISettings() {}

    SPISettings(uint32_t clock, uint8_t bit_order, uint8_t data_mode) {
        (void)clock;
        (void)bit_order;
        (void)data_mode;
    }
};

class SPIClass {
public:
    void begin(void) {}
    void end(void) {}
    void beginTransaction(const SPISettings &settings) { (void)settings; }
    void endTransaction(void) {}
    uint16_t transfer16(uint16_t data) { return data; }
};

static SPIClass SPI;

#endif // __cplusplus

#endif // SPI_COMPAT_H