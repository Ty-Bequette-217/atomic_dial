#include "interface_task.h"
#include "semaphore_guard.h"

InterfaceTask::InterfaceTask(const uint8_t task_core, MotorTask& motor_task, DisplayTask* display_task) : 
        Task("Interface", 2048, 1, task_core),
        motor_task_(motor_task),
        display_task_(display_task),
        configuration_(nullptr) {
    
    mutex_ = xSemaphoreCreateMutex();
    assert(mutex_ != NULL);
}

InterfaceTask::~InterfaceTask() {
    vSemaphoreDelete(mutex_);
}

void InterfaceTask::run() {
    // Initialize UART for debugging
    Serial.begin(921600);
    
    while(1) {
        if (Serial.available()) {
            char c = Serial.read();
            Serial.print(c);
            
            if (c == 'h') {
                log("Smart Knob RP2350B");
                log("Commands:");
                log("  h - help");
                log("  c - start calibration");
            } else if (c == 'c') {
                motor_task_.runCalibration();
                log("Calibration started");
            }
        }
        delay(10);
    }
}

void InterfaceTask::log(const char* msg) {
    SemaphoreGuard lock(mutex_);
    Serial.println(msg);
}

void InterfaceTask::setConfiguration(Configuration* configuration) {
    SemaphoreGuard lock(mutex_);
    configuration_ = configuration;
}
