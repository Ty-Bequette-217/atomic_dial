#pragma once

#include <LittleFS.h>
#include <PacketSerial.h>

#include "freertos_stubs.h"
#include "proto_gen/smartknob.pb.h"

#include "logger.h"

const uint32_t PERSISTENT_CONFIGURATION_VERSION = 1;

class Configuration {
    public:
        Configuration();
        ~Configuration();

        void setLogger(Logger* logger);
        bool loadFromDisk();
        bool saveToDisk();
        PB_PersistentConfiguration get();
        bool setMotorCalibrationAndSave(PB_MotorCalibration& motor_calibration);
        bool setStrainCalibrationAndSave(PB_StrainCalibration& strain_calibration);

    private:
        SemaphoreHandle_t mutex_;

        Logger* logger_ = nullptr;
        bool loaded_ = false;
        PB_PersistentConfiguration pb_buffer_ = {};

        uint8_t buffer_[PB_PersistentConfiguration_size];

        void log(const char* msg);
};

class LittleFSGuard {
    public:
        LittleFSGuard(Logger* logger) : logger_(logger), mounted_(false) {
            // TODO: Properly initialize LittleFS for Arduino Mbed RP2040
            // For now, stub out the filesystem operations
            #if 0
            if (!LittleFS.begin()) {
                if (logger_ != nullptr) {
                    logger_->log("Failed to mount LittleFS");
                }
                return;
            }
            if (logger_ != nullptr) {
                logger_->log("Mounted LittleFS");
            }
            mounted_ = true;
            #else
            if (logger_ != nullptr) {
                logger_->log("LittleFS stub (not initialized)");
            }
            mounted_ = true;
            #endif
        }
        ~LittleFSGuard() {
            if (mounted_) {
                #if 0
                LittleFS.end();
                #endif
                if (logger_ != nullptr) {
                    logger_->log("LittleFS unmounted");
                }
            }
        }
        LittleFSGuard(LittleFSGuard const&)=delete;
        LittleFSGuard& operator=(LittleFSGuard const&)=delete;
        
        bool isMounted() const { return mounted_; }

    private:
        Logger* logger_;
        bool mounted_;
};
