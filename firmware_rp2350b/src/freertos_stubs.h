#pragma once

// FreeRTOS API stubs for Arduino Mbed RP2040
// These are placeholder implementations to allow compilation
// Full FreeRTOS support will be implemented later

#include <Arduino.h>
#include <cstring>

typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;
typedef void* TaskHandle_t;
typedef unsigned int UBaseType_t;
typedef int BaseType_t;

#define pdTRUE    1
#define pdFALSE   0
#define pdPASS    pdTRUE
#define pdFAIL    pdFALSE

#define portMAX_DELAY 0xFFFFFFFF

// Queue stubs
inline QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize) {
    // Allocate a simple FIFO queue in heap
    // For now, just return a non-null pointer
    return (QueueHandle_t)malloc(sizeof(uint32_t));
}

inline BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, uint32_t xTicksToWait) {
    // Stub: queue operations disabled until full implementation
    return pdPASS;
}

inline BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, uint32_t xTicksToWait) {
    // Stub: return false (no messages)
    return pdFALSE;
}

// Semaphore stubs
inline SemaphoreHandle_t xSemaphoreCreateMutex() {
    return (SemaphoreHandle_t)malloc(sizeof(uint32_t));
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, uint32_t xBlockTime) {
    // Stub: always succeed
    return pdPASS;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    // Stub: always succeed
    return pdPASS;
}

inline void vSemaphoreDelete(SemaphoreHandle_t xSemaphore) {
    if (xSemaphore) {
        free(xSemaphore);
    }
}

// Task stubs
inline BaseType_t xTaskCreate(
    void (*pxTaskCode)(void*),
    const char* const pcName,
    uint32_t usStackDepth,
    void* pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t* pxCreatedTask) {
    // Stub: can't create tasks, just mark as created
    if (pxCreatedTask) {
        *pxCreatedTask = (TaskHandle_t)malloc(sizeof(uint32_t));
    }
    return pdPASS;
}

inline BaseType_t xTaskCreatePinnedToCore(
    void (*pxTaskCode)(void*),
    const char* const pcName,
    uint32_t usStackDepth,
    void* pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t* pxCreatedTask,
    BaseType_t xCoreID) {
    // Stub: fallback to xTaskCreate
    return xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask);
}

inline void vTaskDelete(TaskHandle_t xTaskToDelete) {
    // Stub: do nothing
}

inline void vTaskDelay(uint32_t xTicksToDelay) {
    delay(xTicksToDelay);
}
