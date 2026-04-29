#include "motor_feedback.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "SimpleFOC.h"
#include "as5600.h"
#include "board_config.h"

namespace {

constexpr float kTwoPi = 6.2831853071795864769f;
constexpr float kPi = 3.1415926535897932385f;
constexpr float kDegToRad = 0.017453292519943295f;
constexpr float kRadToDeg = 57.29577951308232f;

float wrapAngleDelta(float delta) {
    while (delta > kPi) {
        delta -= kTwoPi;
    }
    while (delta < -kPi) {
        delta += kTwoPi;
    }
    return delta;
}

class AS5600SimpleFOCSensor : public Sensor {
public:
    bool begin() {
        ready_ = as5600_read_raw(&last_raw_);
        if (ready_) {
            filtered_angle_ = rawToRadians(last_raw_);
            last_update_us_ = time_us_64();
            Sensor::init();
        }
        return ready_;
    }

    bool readDegrees(float *degrees) {
        if (degrees == nullptr) {
            return false;
        }

        uint16_t raw_angle;
        if (!as5600_read_raw(&raw_angle)) {
            return false;
        }

        last_raw_ = raw_angle;
        ready_ = true;
        updateFilteredAngle(raw_angle);
        *degrees = filtered_angle_ * kRadToDeg;
        return true;
    }

    float filteredVelocity() const {
        return filtered_velocity_;
    }

protected:
    float getSensorAngle() override {
        uint16_t raw_angle;
        if (as5600_read_raw(&raw_angle)) {
            last_raw_ = raw_angle;
            ready_ = true;
            updateFilteredAngle(raw_angle);
        }

        return filtered_angle_;
    }

private:
    void updateFilteredAngle(uint16_t raw_angle) {
        const float raw_radians = rawToRadians(raw_angle);
        const float delta = wrapAngleDelta(raw_radians - filtered_angle_);
        const uint64_t now_us = time_us_64();

        const float filtered_delta = MOTOR_ENCODER_FILTER_ALPHA * delta;
        filtered_angle_ += filtered_delta;
        while (filtered_angle_ >= kTwoPi) {
            filtered_angle_ -= kTwoPi;
        }
        while (filtered_angle_ < 0.0f) {
            filtered_angle_ += kTwoPi;
        }

        if (last_update_us_ != 0 && now_us > last_update_us_) {
            const float dt = (float)(now_us - last_update_us_) * 1.0e-6f;
            if (dt > 0.0f) {
                const float instantaneous_velocity = filtered_delta / dt;
                filtered_velocity_ +=
                    MOTOR_VELOCITY_FILTER_ALPHA * (instantaneous_velocity - filtered_velocity_);
                if (fabsf(filtered_velocity_) < MOTOR_VELOCITY_NOISE_DEADBAND_RAD_S) {
                    filtered_velocity_ = 0.0f;
                }
            }
        }

        last_update_us_ = now_us;
    }

    static float rawToRadians(uint16_t raw_angle) {
        return ((float)(raw_angle & 0x0FFF) * kTwoPi) / 4096.0f;
    }

    uint16_t last_raw_ = 0;
    float filtered_angle_ = 0.0f;
    float filtered_velocity_ = 0.0f;
    uint64_t last_update_us_ = 0;
    bool ready_ = false;
};

AS5600SimpleFOCSensor sensor;
BLDCMotor motor(MOTOR_DEFAULT_POLE_PAIRS);
BLDCDriver6PWM driver(
    MOTOR_PHASE_U_HIGH_PIN, MOTOR_PHASE_U_LOW_PIN,
    MOTOR_PHASE_V_HIGH_PIN, MOTOR_PHASE_V_LOW_PIN,
    MOTOR_PHASE_W_HIGH_PIN, MOTOR_PHASE_W_LOW_PIN
);

ui_mode_t current_mode = UI_MODE_UNBOUNDED_NO_DETENTS;
bool initialized = false;
float latest_angle_rad = 0.0f;
float latest_velocity_rad_s = 0.0f;

bool modeUsesActiveTorque(ui_mode_t mode) {
    switch (mode) {
        case UI_MODE_SOFT_DETENTS:
        case UI_MODE_STRONG_DETENTS:
            return true;
        case UI_MODE_UNBOUNDED_NO_DETENTS:
        case UI_MODE_COUNT:
        default:
            return false;
    }
}

bool initFOCWithRetry(void) {
    if (motor.initFOC()) {
        return true;
    }

    printf("BLDC motor FOC init retrying with stronger alignment\n");
    motor.disable();
    sleep_ms(100);
    motor.enable();
    motor.voltage_sensor_align = MOTOR_VOLTAGE_LIMIT;
    return motor.initFOC();
}

float detentStrengthForMode(ui_mode_t mode) {
    switch (mode) {
        case UI_MODE_SOFT_DETENTS:
            return MOTOR_SOFT_DETENT_STRENGTH;
        case UI_MODE_STRONG_DETENTS:
            return MOTOR_STRONG_DETENT_STRENGTH;
        case UI_MODE_UNBOUNDED_NO_DETENTS:
        case UI_MODE_COUNT:
        default:
            return 0.0f;
    }
}

float detentDampingForMode(ui_mode_t mode) {
    switch (mode) {
        case UI_MODE_SOFT_DETENTS:
            return MOTOR_SOFT_DETENT_DAMPING;
        case UI_MODE_STRONG_DETENTS:
            return MOTOR_STRONG_DETENT_DAMPING;
        case UI_MODE_UNBOUNDED_NO_DETENTS:
        case UI_MODE_COUNT:
        default:
            return 0.0f;
    }
}

float detentCenterDeadbandForMode(ui_mode_t mode) {
    switch (mode) {
        case UI_MODE_SOFT_DETENTS:
            return MOTOR_SOFT_DETENT_CENTER_DEADBAND_DEG * kDegToRad;
        case UI_MODE_STRONG_DETENTS:
            return MOTOR_STRONG_DETENT_CENTER_DEADBAND_DEG * kDegToRad;
        case UI_MODE_UNBOUNDED_NO_DETENTS:
        case UI_MODE_COUNT:
        default:
            return 0.0f;
    }
}

float detentFeedbackVoltage(float angle, float velocity) {
    const float strength = detentStrengthForMode(current_mode);
    if (strength <= 0.0f) {
        return 0.0f;
    }

    const float spacing = MOTOR_DETENT_SPACING_DEG * kDegToRad;
    const float detent_center = roundf(angle / spacing) * spacing;
    float error = wrapAngleDelta(detent_center - angle);

    const float center_deadband = detentCenterDeadbandForMode(current_mode);
    if (fabsf(error) <= center_deadband) {
        error = 0.0f;
    } else {
        error -= copysignf(center_deadband, error);
    }

    const float half_spacing = spacing * 0.5f;
    float voltage = (strength * (error / half_spacing))
                  - (detentDampingForMode(current_mode) * velocity);
    if (voltage > MOTOR_VOLTAGE_LIMIT) {
        voltage = MOTOR_VOLTAGE_LIMIT;
    } else if (voltage < -MOTOR_VOLTAGE_LIMIT) {
        voltage = -MOTOR_VOLTAGE_LIMIT;
    }

    return voltage;
}

void applyModeState(ui_mode_t mode) {
    if (!initialized) {
        return;
    }

    if (modeUsesActiveTorque(mode)) {
        motor.enable();
    } else {
        motor.disable();
    }
}

} // namespace

extern "C" bool motor_feedback_init(ui_mode_t initial_mode) {
    current_mode = initial_mode;

    gpio_init(MOTOR_DIAG_PIN);
    gpio_set_dir(MOTOR_DIAG_PIN, GPIO_IN);
    gpio_pull_up(MOTOR_DIAG_PIN);

    if (!sensor.begin()) {
        printf("Motor feedback AS5600 sensor init failed\n");
        return false;
    }

    driver.voltage_power_supply = MOTOR_SUPPLY_VOLTAGE;
    driver.voltage_limit = MOTOR_VOLTAGE_LIMIT;
    if (!driver.init()) {
        printf("TMC6300 6PWM driver init failed\n");
        return false;
    }

    motor.linkSensor(&sensor);
    motor.linkDriver(&driver);
    motor.torque_controller = TorqueControlType::voltage;
    motor.controller = MotionControlType::torque;
    motor.voltage_limit = MOTOR_VOLTAGE_LIMIT;
    motor.voltage_sensor_align = 0.8f;

    if (!motor.init()) {
        printf("BLDC motor init failed\n");
        return false;
    }
    if (!initFOCWithRetry()) {
        printf("BLDC motor FOC init failed\n");
        return false;
    }

    latest_angle_rad = sensor.getAngle();
    latest_velocity_rad_s = 0.0f;
    initialized = true;
    applyModeState(current_mode);
    return true;
}

extern "C" void motor_feedback_set_mode(ui_mode_t mode) {
    current_mode = mode;
    applyModeState(mode);
    printf("Motor feedback mode: %d\n", (int)mode);
}

extern "C" void motor_feedback_update(void) {
    if (!initialized) {
        return;
    }

    if (modeUsesActiveTorque(current_mode)) {
        motor.loopFOC();
    }
    const float angle = sensor.getAngle();
    const float velocity = sensor.filteredVelocity();
    latest_angle_rad = angle;
    latest_velocity_rad_s = velocity;
    float voltage = 0.0f;

    if (modeUsesActiveTorque(current_mode)) {
        voltage = detentFeedbackVoltage(angle, velocity);
        motor.move(voltage);
    }

    static uint32_t next_debug_ms = 0;
    const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
    if (current_mode != UI_MODE_UNBOUNDED_NO_DETENTS && now_ms >= next_debug_ms) {
        next_debug_ms = now_ms + 500;
        printf("Detent mode=%d angle=%.1f vel=%.2f voltage=%.2f\n",
               (int)current_mode,
               angle * kRadToDeg,
               velocity,
               voltage);
    }
}

extern "C" bool motor_feedback_get_angle_degrees(float *degrees) {
    if (!initialized || degrees == nullptr) {
        return false;
    }

    *degrees = fmodf(latest_angle_rad * kRadToDeg, 360.0f);
    if (*degrees < 0.0f) {
        *degrees += 360.0f;
    }
    return true;
}
