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

struct HapticModeProfile {
    bool uses_active_torque;
    int32_t min_position;       
    int32_t max_position;       
    float position_width_rad;   
    float detent_strength;      
    float endstop_strength;     
    float damping_factor;       
    float snap_point;           
    float snap_point_bias;      
    float dead_zone_rad;        
};

constexpr HapticModeProfile kHapticProfiles[UI_MODE_COUNT] = {
    { false, 1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f },
    { true, 1, 0, 15.0f * kDegToRad, 1.2f, 0.0f, 0.04f, 0.55f, 0.05f, 0.5f * kDegToRad },
    { true, 0, 10, 20.0f * kDegToRad, 2.8f, 15.0f, 0.06f, 0.55f, 0.10f, 0.8f * kDegToRad },
    { true, 0, 1, 140.0f * kDegToRad, 3.8f, 60.0f, 0.08f, 0.60f, 0.15f, 1.0f * kDegToRad }
};

ui_mode_t current_mode = UI_MODE_UNBOUNDED_NO_DETENTS;
bool initialized = false;
float unwrapped_angle_rad = 0.0f;
float last_wrapped_rad = 0.0f;
float velocity_rad_s = 0.0f;

float haptic_logical_offset_rad = 0.0f;
float haptic_center_rad = 0.0f;
int32_t haptic_current_position = 0;
float haptic_idle_velocity_ewma = 0.0f;
uint32_t last_idle_start_ms = 0;

float wrapAngleDelta(float delta) {
    while (delta > kPi) delta -= kTwoPi;
    while (delta < -kPi) delta += kTwoPi;
    return delta;
}

float clampFloat(float value, float min_value, float max_value) {
    return fminf(fmaxf(value, min_value), max_value);
}

void initializeHapticState(ui_mode_t mode, float unwrapped_angle) {
    haptic_logical_offset_rad = unwrapped_angle;
    haptic_current_position = 0; 
    haptic_center_rad = 0.0f;
    haptic_idle_velocity_ewma = 0.0f;
    last_idle_start_ms = 0;
}

class AS5600Sensor : public Sensor {
public:
    bool begin() {
        bool ok = as5600_read_raw(&last_raw_);
        if (ok) {
            filtered_angle_ = ((float)(last_raw_ & 0x0FFF) * kTwoPi) / 4096.0f;
            last_update_us_ = time_us_64();
            Sensor::init();
        }
        return ok;
    }
protected:
    float getSensorAngle() override {
        uint16_t raw;
        if (as5600_read_raw(&raw)) {
            float rad = ((float)(raw & 0x0FFF) * kTwoPi) / 4096.0f;
            float delta = wrapAngleDelta(rad - filtered_angle_);
            filtered_angle_ += MOTOR_ENCODER_FILTER_ALPHA * delta;
            uint64_t now = time_us_64();
            float dt = (float)(now - last_update_us_) * 1e-6f;
            if (dt > 0.0001f) {
                float inst_vel = (MOTOR_ENCODER_FILTER_ALPHA * delta) / dt;
                filtered_velocity_ += MOTOR_VELOCITY_FILTER_ALPHA * (inst_vel - filtered_velocity_);
                last_update_us_ = now;
            }
        }
        return filtered_angle_;
    }
    uint16_t last_raw_ = 0;
    float filtered_angle_ = 0.0f;
    float filtered_velocity_ = 0.0f;
    uint64_t last_update_us_ = 0;
};

AS5600Sensor sensor;
BLDCMotor motor(7); // Explicit pole pairs
BLDCDriver6PWM driver(MOTOR_PHASE_U_HIGH_PIN, MOTOR_PHASE_U_LOW_PIN, 
                      MOTOR_PHASE_V_HIGH_PIN, MOTOR_PHASE_V_LOW_PIN, 
                      MOTOR_PHASE_W_HIGH_PIN, MOTOR_PHASE_W_LOW_PIN);

float calculateSmartKnobVoltage(float logical_angle, float velocity, const HapticModeProfile& profile) {
    if (!profile.uses_active_torque || profile.position_width_rad <= 0.0f) return 0.0f;

    bool has_bounds = profile.min_position <= profile.max_position;
    float angle_to_center = logical_angle - haptic_center_rad;

    // --- State Transitions ---
    float snap_rad = profile.position_width_rad * profile.snap_point;
    float bias_rad = profile.position_width_rad * profile.snap_point_bias;
    float snap_inc = snap_rad + (haptic_current_position >= 0 ? -bias_rad : bias_rad);
    float snap_dec = -snap_rad + (haptic_current_position <= 0 ? bias_rad : -bias_rad);

    if (angle_to_center > snap_inc && (!has_bounds || haptic_current_position < profile.max_position)) {
        haptic_center_rad += profile.position_width_rad;
        angle_to_center -= profile.position_width_rad;
        haptic_current_position++;
    } else if (angle_to_center < snap_dec && (!has_bounds || haptic_current_position > profile.min_position)) {
        haptic_center_rad -= profile.position_width_rad;
        angle_to_center += profile.position_width_rad;
        haptic_current_position--;
    }

    // --- Absolute Center Correction (Scott's Logic) ---
    // Clamping ensures haptic_center_rad stays exactly on the logical step
    if (has_bounds) {
        haptic_center_rad = clampFloat(haptic_center_rad, 
                                       (float)profile.min_position * profile.position_width_rad, 
                                       (float)profile.max_position * profile.position_width_rad);
    }

    // --- End-Stop Detection ---
    bool out_of_bounds = has_bounds && (
        (angle_to_center > 0.0f && haptic_current_position == profile.max_position) ||
        (angle_to_center < 0.0f && haptic_current_position == profile.min_position)
    );

    // --- Idle Drift Lock ---
    haptic_idle_velocity_ewma = velocity * 0.001f + haptic_idle_velocity_ewma * 0.999f;
    if (fabsf(haptic_idle_velocity_ewma) > 0.05f) {
        last_idle_start_ms = 0;
    } else if (last_idle_start_ms == 0) {
        last_idle_start_ms = to_ms_since_boot(get_absolute_time());
    }

    // Only "relax" if in-bounds. If out of bounds, wall stays rigid
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (!out_of_bounds && last_idle_start_ms > 0 && (now - last_idle_start_ms) > 500 &&
        fabsf(logical_angle - haptic_center_rad) < (5.0f * kDegToRad)) {
        haptic_center_rad = logical_angle * 0.0005f + haptic_center_rad * 0.9995f;
    }

    // --- Force Calculation ---
    const float strength = out_of_bounds ? profile.endstop_strength : profile.detent_strength;
    const float dead_zone = out_of_bounds ? 0.0f : clampFloat(angle_to_center, -profile.dead_zone_rad, profile.dead_zone_rad);
    
    float p_force = strength * (-(angle_to_center - dead_zone));
    float d_force = -(strength * profile.damping_factor * velocity);

    if (out_of_bounds) {
        bool moving_deeper = (angle_to_center * velocity) > 0;
        if (!moving_deeper) {
            // SNAP-BACK BOOST: If user lets go, double the spring force and kill the brake
            p_force *= 1.5f; 
            d_force = 0.0f; 
        } else {
            d_force *= 4.0f; // Heavy resistance moving in
        }
    }

    return clampFloat(p_force + d_force, -MOTOR_VOLTAGE_LIMIT, MOTOR_VOLTAGE_LIMIT);
}

} // namespace

extern "C" bool motor_feedback_init(ui_mode_t initial_mode) {
    current_mode = initial_mode;
    gpio_init(MOTOR_DIAG_PIN); gpio_set_dir(MOTOR_DIAG_PIN, GPIO_IN); gpio_pull_up(MOTOR_DIAG_PIN);
    if (!sensor.begin()) return false;
    driver.voltage_power_supply = MOTOR_SUPPLY_VOLTAGE;
    driver.voltage_limit = MOTOR_VOLTAGE_LIMIT;
    if (!driver.init()) return false;
    motor.linkSensor(&sensor); motor.linkDriver(&driver);
    motor.torque_controller = TorqueControlType::voltage;
    motor.controller = MotionControlType::torque;
    
    // CRITICAL: Force a high-power alignment for efficient push-back
    motor.voltage_sensor_align = 3.0f; 
    if (!motor.init() || !motor.initFOC()) return false;
    
    last_wrapped_rad = sensor.getAngle();
    unwrapped_angle_rad = last_wrapped_rad;
    initializeHapticState(current_mode, unwrapped_angle_rad);
    initialized = true;
    if (kHapticProfiles[current_mode].uses_active_torque) motor.enable();
    return true;
}

extern "C" void motor_feedback_set_mode(ui_mode_t mode) {
    current_mode = mode;
    if (initialized) {
        initializeHapticState(mode, unwrapped_angle_rad);
        if (kHapticProfiles[mode].uses_active_torque) motor.enable();
        else motor.disable();
    }
}

extern "C" void motor_feedback_update(void) {
    if (!initialized) return;
    if (kHapticProfiles[current_mode].uses_active_torque) motor.loopFOC();
    else sensor.update();
    float rad = sensor.getAngle();
    float delta = wrapAngleDelta(rad - last_wrapped_rad);
    unwrapped_angle_rad += delta;
    last_wrapped_rad = rad;
    velocity_rad_s = sensor.getVelocity();
    if (kHapticProfiles[current_mode].uses_active_torque) {
        float logical = unwrapped_angle_rad - haptic_logical_offset_rad;
        motor.move(calculateSmartKnobVoltage(logical, velocity_rad_s, kHapticProfiles[current_mode]));
    }
}

extern "C" void motor_feedback_play_click(bool press) {
    if (!initialized) return;
    float strength = press ? 6.0f : 2.5f; 
    motor.move(strength);
    for (int i = 0; i < 3; i++) { motor.loopFOC(); sleep_us(1000); }
    motor.move(-strength);
    for (int i = 0; i < 3; i++) { motor.loopFOC(); sleep_us(1000); }
    motor.move(0);
}

extern "C" bool motor_feedback_get_angle_degrees(float *degrees) {
    if (!initialized || degrees == nullptr) return false;
    *degrees = fmodf((unwrapped_angle_rad - haptic_logical_offset_rad) * kRadToDeg, 360.0f);
    if (*degrees < 0.0f) *degrees += 360.0f;
    return true;
}

extern "C" float motor_feedback_get_velocity(void) { return initialized ? velocity_rad_s : 0.0f; }
extern "C" bool motor_feedback_get_logical_position(int32_t *position) {
    if (!initialized || position == nullptr) return false;
    *position = haptic_current_position;
    return true;
}
extern "C" bool motor_feedback_get_bounds(int32_t *min_pos, int32_t *max_pos) {
    if (!initialized || min_pos == nullptr || max_pos == nullptr) return false;
    *min_pos = kHapticProfiles[current_mode].min_position;
    *max_pos = kHapticProfiles[current_mode].max_position;
    return true;
}