#pragma once

#include <AceButton.h>
#include <Arduino.h>

#include "configuration.h"
#include "display_task.h"
#include "freertos_stubs.h"
#include "logger.h"
#include "motor_task.h"
#include "task.h"
#include "semaphore_guard.h"

class InterfaceTask : public Task<InterfaceTask>, public Logger {
    friend class Task<InterfaceTask>;

    public:
        InterfaceTask(const uint8_t task_core, MotorTask& motor_task, DisplayTask* display_task);
        virtual ~InterfaceTask();

        void log(const char* msg) override;
        void setConfiguration(Configuration* configuration);

    protected:
        void run();

    private:
        MotorTask& motor_task_;
        DisplayTask* display_task_;
        char buf_[128];
        SemaphoreHandle_t mutex_;
        Configuration* configuration_;
};
