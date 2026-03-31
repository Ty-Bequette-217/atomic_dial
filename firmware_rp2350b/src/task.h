#pragma once

#include <Arduino.h>
#include "freertos_stubs.h"

// Static polymorphic abstract base class for a FreeRTOS task using CRTP pattern. Concrete implementations
// should implement a run() method.
template<class T>
class Task {
    public:
        Task(const char* name, uint32_t stackDepth, UBaseType_t priority, const BaseType_t coreId = 0) : 
                name_ { name },
                stackDepth_ {stackDepth},
                priority_ { priority },
                coreId_ { coreId },
                taskHandle_ { nullptr }
        {}
        virtual ~Task() {};

        TaskHandle_t getHandle() {
            return taskHandle_;
        }

        void begin() {
            // TODO: Implement FreeRTOS task creation for RP2040
            // For now: start run() directly or use Arduino thread if available
            static_cast<T*>(this)->run();
        }

    protected:
        const char* name_;
        uint32_t stackDepth_;
        UBaseType_t priority_;
        BaseType_t coreId_;
        TaskHandle_t taskHandle_;
};
