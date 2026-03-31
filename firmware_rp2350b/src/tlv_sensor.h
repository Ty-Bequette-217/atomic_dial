#pragma once

#include <SimpleFOC.h>

class TlvSensor : public Sensor {
    public:
        TlvSensor();
        void init(TwoWire* wire, bool invert);
        float getSensorAngle();

    private:
        TwoWire* wire_;
        bool invert_;
        float x_;
        float y_;
        uint32_t last_update_;
        uint8_t frame_counts_[8] = {};
        uint8_t cur_frame_count_index_ = 0;
        bool error_ = false;
};
