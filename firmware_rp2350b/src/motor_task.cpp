#include <SimpleFOC.h>

#include "motor_task.h"
#if SENSOR_MT6701
#include "mt6701_sensor.h"
#elif SENSOR_TLV
#include "tlv_sensor.h"
#elif SENSOR_MAQ430
#include "maq430_sensor.h"
#endif

#include "motors/motor_config.h"
#include "util.h"

static const float DEAD_ZONE_DETENT_PERCENT = 0.2;
static const float DEAD_ZONE_RAD = 1 * _PI / 180;

static const float IDLE_VELOCITY_EWMA_ALPHA = 0.001;
static const float IDLE_VELOCITY_RAD_PER_SEC = 0.05;
static const uint32_t IDLE_CORRECTION_DELAY_MILLIS = 500;
static const float IDLE_CORRECTION_MAX_ANGLE_RAD = 5 * PI / 180;
static const float IDLE_CORRECTION_RATE_ALPHA = 0.0005;


MotorTask::MotorTask(const uint8_t task_core, Configuration& configuration) : Task("Motor", 4000, 1, task_core), configuration_(configuration) {
    queue_ = xQueueCreate(5, sizeof(Command));
    assert(queue_ != NULL);
}

MotorTask::~MotorTask() {}


#if SENSOR_TLV
    TlvSensor encoder = TlvSensor();
#elif SENSOR_MT6701
    MT6701Sensor encoder = MT6701Sensor();
#elif SENSOR_MAQ430
    MagneticSensorSPI encoder = MagneticSensorSPI(MAQ430_SPI, PIN_MAQ_SS);
#endif

void MotorTask::run() {

    driver.voltage_power_supply = 5;
    driver.init();

    #if SENSOR_TLV
    encoder.init(&Wire, false);
    #elif SENSOR_MT6701
    encoder.init();
    #elif SENSOR_MAQ430
    SPIClass* spi = new SPIClass(HSPI);
    spi->begin(PIN_MAQ_SCK, PIN_MAQ_MISO, PIN_MAQ_MOSI, PIN_MAQ_SS);
    encoder.init(spi);
    #endif

    motor.linkDriver(&driver);

    motor.controller = MotionControlType::torque;
    motor.voltage_limit = FOC_VOLTAGE_LIMIT;
    motor.velocity_limit = 10000;
    motor.linkSensor(&encoder);

    motor.PID_velocity.P = FOC_PID_P;
    motor.PID_velocity.I = FOC_PID_I;
    motor.PID_velocity.D = FOC_PID_D;
    motor.PID_velocity.output_ramp = FOC_PID_OUTPUT_RAMP;
    motor.PID_velocity.limit = FOC_PID_LIMIT;

    #ifdef FOC_LPF
    motor.LPF_angle.Tf = FOC_LPF;
    #endif

    motor.init();

    encoder.update();
    delay(10);

    PB_PersistentConfiguration c = configuration_.get();
    motor.pole_pairs = c.motor.calibrated ? c.motor.pole_pairs : 7;
    motor.initFOC(c.motor.zero_electrical_offset, c.motor.direction_cw ? Direction::CW : Direction::CCW);

    motor.monitor_downsample = 0;

    float current_detent_center = motor.shaft_angle;
    PB_SmartKnobConfig config = {
        .position = 0,
        .sub_position_unit = 0,
        .position_nonce = 0,
        .min_position = 0,
        .max_position = 1,
        .position_width_radians = 60 * _PI / 180,
        .detent_strength_unit = 0,
    };
    int32_t current_position = 0;
    float latest_sub_position_unit = 0;

    float idle_check_velocity_ewma = 0;
    uint32_t last_idle_start = 0;
    uint32_t last_publish = 0;

    while (1) {
        motor.loopFOC();

        Command command;
        if (xQueueReceive(queue_, &command, 0) == pdTRUE) {
            switch (command.command_type) {
                case CommandType::CALIBRATE:
                    calibrate();
                    break;
                case CommandType::CONFIG: {
                    PB_SmartKnobConfig& new_config = command.data.config;
                    if (new_config.detent_strength_unit < 0) {
                        log("Ignoring invalid config: detent_strength_unit cannot be negative");
                        break;
                    }
                    if (new_config.endstop_strength_unit < 0) {
                        log("Ignoring invalid config: endstop_strength_unit cannot be negative");
                        break;
                    }
                    if (new_config.snap_point < 0.5) {
                        log("Ignoring invalid config: snap_point must be >= 0.5 for stability");
                        break;
                    }
                    if (new_config.detent_positions_count > COUNT_OF(new_config.detent_positions)) {
                        log("Ignoring invalid config: detent_positions_count is too large");
                        break;
                    }
                    if (new_config.snap_point_bias < 0) {
                        log("Ignoring invalid config: snap_point_bias cannot be negative or there is risk of instability");
                        break;
                    }

                    bool position_updated = false;
                    if (new_config.position != config.position
                            || new_config.sub_position_unit != config.sub_position_unit
                            || new_config.position_nonce != config.position_nonce) {
                        log("applying position change");
                        current_position = new_config.position;
                        position_updated = true;
                    }

                    if (new_config.min_position <= new_config.max_position) {
                        if (current_position < new_config.min_position) {
                            current_position = new_config.min_position;
                            log("adjusting position to min");
                        } else if (current_position > new_config.max_position) {
                            current_position = new_config.max_position;
                            log("adjusting position to max");
                        }
                    }

                    if (position_updated || new_config.position_width_radians != config.position_width_radians) {
                        log("adjusting detent center");
                        float new_sub_position = position_updated ? new_config.sub_position_unit : latest_sub_position_unit;
                        #if SK_INVERT_ROTATION
                            float shaft_angle = -motor.shaft_angle;
                        #else
                            float shaft_angle = motor.shaft_angle;
                        #endif
                        current_detent_center = shaft_angle + new_sub_position * new_config.position_width_radians;
                    }
                    config = new_config;
                    log("Got new config");

                    const float derivative_lower_strength = config.detent_strength_unit * 0.08;
                    const float derivative_upper_strength = config.detent_strength_unit * 0.02;
                    const float derivative_position_width_lower = radians(3);
                    const float derivative_position_width_upper = radians(8);
                    const float raw = derivative_lower_strength + (derivative_upper_strength - derivative_lower_strength)/(derivative_position_width_upper - derivative_position_width_lower)*(config.position_width_radians - derivative_position_width_lower);
                    motor.PID_velocity.D = config.detent_positions_count > 0 ? 0 : CLAMP(
                        raw,
                        min(derivative_lower_strength, derivative_upper_strength),
                        max(derivative_lower_strength, derivative_upper_strength)
                    );
                    break;
                }
                case CommandType::HAPTIC: {
                    float strength = command.data.haptic.press ? 5 : 1.5;
                    motor.move(strength);
                    for (uint8_t i = 0; i < 3; i++) {
                        motor.loopFOC();
                        delay(1);
                    }
                    motor.move(-strength);
                    for (uint8_t i = 0; i < 3; i++) {
                        motor.loopFOC();
                        delay(1);
                    }
                    motor.move(0);
                    motor.loopFOC();
                    break;
                }
            }
        }

        idle_check_velocity_ewma = motor.shaft_velocity * IDLE_VELOCITY_EWMA_ALPHA + idle_check_velocity_ewma * (1 - IDLE_VELOCITY_EWMA_ALPHA);
        if (fabsf(idle_check_velocity_ewma) > IDLE_VELOCITY_RAD_PER_SEC) {
            last_idle_start = 0;
        } else {
            if (last_idle_start == 0) {
                last_idle_start = millis();
            }
        }
        if (last_idle_start > 0 && millis() - last_idle_start > IDLE_CORRECTION_DELAY_MILLIS && fabsf(motor.shaft_angle - current_detent_center) < IDLE_CORRECTION_MAX_ANGLE_RAD) {
            current_detent_center = motor.shaft_angle * IDLE_CORRECTION_RATE_ALPHA + current_detent_center * (1 - IDLE_CORRECTION_RATE_ALPHA);
        }

        float angle_to_detent_center = motor.shaft_angle - current_detent_center;
        #if SK_INVERT_ROTATION
            angle_to_detent_center = -motor.shaft_angle - current_detent_center;
        #endif

        float snap_point_radians = config.position_width_radians * config.snap_point;
        float bias_radians = config.position_width_radians * config.snap_point_bias;
        float snap_point_radians_decrease = snap_point_radians + (current_position <= 0 ? bias_radians : -bias_radians);
        float snap_point_radians_increase = -snap_point_radians + (current_position >= 0 ? -bias_radians : bias_radians); 

        int32_t num_positions = config.max_position - config.min_position + 1;
        if (angle_to_detent_center > snap_point_radians_decrease && (num_positions <= 0 || current_position > config.min_position)) {
            current_detent_center += config.position_width_radians;
            angle_to_detent_center -= config.position_width_radians;
            current_position--;
        } else if (angle_to_detent_center < snap_point_radians_increase && (num_positions <= 0 || current_position < config.max_position)) {
            current_detent_center -= config.position_width_radians;
            angle_to_detent_center += config.position_width_radians;
            current_position++;
        }

        latest_sub_position_unit = -angle_to_detent_center / config.position_width_radians;

        float dead_zone_adjustment = CLAMP(
            angle_to_detent_center,
            fmaxf(-config.position_width_radians*DEAD_ZONE_DETENT_PERCENT, -DEAD_ZONE_RAD),
            fminf(config.position_width_radians*DEAD_ZONE_DETENT_PERCENT, DEAD_ZONE_RAD));

        bool out_of_bounds = num_positions > 0 && ((angle_to_detent_center > 0 && current_position == config.min_position) || (angle_to_detent_center < 0 && current_position == config.max_position));
        motor.PID_velocity.limit = 10;
        motor.PID_velocity.P = out_of_bounds ? config.endstop_strength_unit * 4 : config.detent_strength_unit * 4;

        if (fabsf(motor.shaft_velocity) > 60) {
            motor.move(0);
        } else {
            float input = -angle_to_detent_center + dead_zone_adjustment;
            if (!out_of_bounds && config.detent_positions_count > 0) {
                bool in_detent = false;
                for (uint8_t i = 0; i < config.detent_positions_count; i++) {
                    if (config.detent_positions[i] == current_position) {
                        in_detent = true;
                        break;
                    }
                }
                if (!in_detent) {
                    input = 0;
                }
            }
            float torque = motor.PID_velocity(input);
            #if SK_INVERT_ROTATION
                torque = -torque;
            #endif
            motor.move(torque);
        }

        if (millis() - last_publish > 5) {
            publish({
                .current_position = current_position,
                .sub_position_unit = latest_sub_position_unit,
                .has_config = true,
                .config = config,
            });
            last_publish = millis();
        }

        delay(1);
    }
}

void MotorTask::publish(const PB_SmartKnobState& state) {
    for (auto listener : listeners_) {
        PB_SmartKnobState state_copy = state;
        xQueueSend(listener, &state_copy, 0);
    }
}

void MotorTask::addListener(QueueHandle_t queue) {
    listeners_.push_back(queue);
}

void MotorTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void MotorTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}

void MotorTask::playHaptic(bool press) {
    Command command;
    command.command_type = CommandType::HAPTIC;
    command.data.haptic.press = press;
    xQueueSend(queue_, &command, 0);
}

void MotorTask::setConfig(const PB_SmartKnobConfig& config) {
    Command command;
    command.command_type = CommandType::CONFIG;
    command.data.config = config;
    xQueueSend(queue_, &command, 0);
}

void MotorTask::runCalibration() {
    Command command;
    command.command_type = CommandType::CALIBRATE;
    xQueueSend(queue_, &command, 0);
}

void MotorTask::calibrate() {
    log("Starting motor calibration");
}

void MotorTask::checkSensorError() {
}
