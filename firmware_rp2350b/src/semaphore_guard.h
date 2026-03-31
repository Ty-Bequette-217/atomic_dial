#pragma once

#include "freertos_stubs.h"

class SemaphoreGuard {
    public:
        SemaphoreGuard(SemaphoreHandle_t semaphore) : semaphore_(semaphore) {
            if (semaphore_) {
                xSemaphoreTake(semaphore_, portMAX_DELAY);
            }
        }
        ~SemaphoreGuard() {
            if (semaphore_) {
                xSemaphoreGive(semaphore_);
            }
        }
        SemaphoreGuard(SemaphoreGuard const&) = delete;
        SemaphoreGuard& operator=(SemaphoreGuard const&) = delete;

    private:
        SemaphoreHandle_t semaphore_;
};
