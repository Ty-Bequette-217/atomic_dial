#pragma once

#include <Arduino.h>

class Logger {
    public:
        virtual ~Logger() = default;
        virtual void log(const char* msg) = 0;
};
