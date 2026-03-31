#include "tlv_sensor.h"

TlvSensor::TlvSensor() : x_(0), y_(0), last_update_(0), error_(false) {}

void TlvSensor::init(TwoWire* wire, bool invert) {
    wire_ = wire;
    invert_ = invert;
}

float TlvSensor::getSensorAngle() {
    uint32_t now = micros();
    if (now - last_update_ > 50) {
        last_update_ = now;
    }
    float rad = (invert_ ? -1 : 1) * atan2f(y_, x_);
    if (rad < 0) {
        rad += 2*PI;
    }
    return rad;
}
