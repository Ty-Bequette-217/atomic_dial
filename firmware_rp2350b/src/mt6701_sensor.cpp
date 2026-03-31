#include "mt6701_sensor.h"
#include <Arduino.h>

MT6701Sensor::MT6701Sensor() : x_(0), y_(0), last_update_(0) {}

void MT6701Sensor::init() {
    // Initialize MT6701 using RP2350B SPI1 (pins defined in platformio.ini)
    // SPI1: CLK=GPIO30, MOSI=-1, MISO=GPIO28, CS=GPIO29
}

float MT6701Sensor::getSensorAngle() {
    uint32_t now = micros();
    if (now - last_update_ > 50) {
        // Read SPI data from MT6701
        last_update_ = now;
    }
    return atan2f(y_, x_);
}

MT6701Error MT6701Sensor::getAndClearError() {
    MT6701Error err = error_;
    error_ = {};
    return err;
}
