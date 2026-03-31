#pragma once

#include <SimpleFOC.h>

struct MT6701Error {
    bool error;
    uint8_t received_crc;
    uint8_t calculated_crc;
};

class MT6701Sensor : public Sensor {
    public:
        MT6701Sensor();

        void init();
        float getSensorAngle();

        MT6701Error getAndClearError();
    private:
        float x_;
        float y_;
        uint32_t last_update_;

        MT6701Error error_ = {};
};
