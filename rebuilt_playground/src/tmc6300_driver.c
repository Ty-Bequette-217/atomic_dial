#include "tmc6300_driver.h"

#include "board_config.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

static bool tmc6300_initialized;

static void tmc6300_write_level(uint gpio, uint16_t level) {
    uint16_t clamped = (level > MOTOR_PWM_WRAP) ? MOTOR_PWM_WRAP : level;
    pwm_set_gpio_level(gpio, clamped);
}

void tmc6300_init(void) {
    static const uint motor_pins[] = {
        MOTOR_PHASE_U_HIGH_PIN,
        MOTOR_PHASE_U_LOW_PIN,
        MOTOR_PHASE_V_HIGH_PIN,
        MOTOR_PHASE_V_LOW_PIN,
        MOTOR_PHASE_W_HIGH_PIN,
        MOTOR_PHASE_W_LOW_PIN,
    };

    for (uint i = 0; i < (sizeof motor_pins / sizeof motor_pins[0]); ++i) {
        uint gpio = motor_pins[i];
        gpio_set_function(gpio, GPIO_FUNC_PWM);
        uint slice = pwm_gpio_to_slice_num(gpio);
        pwm_set_wrap(slice, MOTOR_PWM_WRAP);
        pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), 0);
        pwm_set_enabled(slice, true);
    }

    gpio_init(MOTOR_DIAG_PIN);
    gpio_set_dir(MOTOR_DIAG_PIN, GPIO_IN);
    gpio_pull_up(MOTOR_DIAG_PIN);

    tmc6300_initialized = true;
}

void tmc6300_disable_all(void) {
    if (!tmc6300_initialized) {
        return;
    }

    tmc6300_write_level(MOTOR_PHASE_U_HIGH_PIN, 0);
    tmc6300_write_level(MOTOR_PHASE_U_LOW_PIN, 0);
    tmc6300_write_level(MOTOR_PHASE_V_HIGH_PIN, 0);
    tmc6300_write_level(MOTOR_PHASE_V_LOW_PIN, 0);
    tmc6300_write_level(MOTOR_PHASE_W_HIGH_PIN, 0);
    tmc6300_write_level(MOTOR_PHASE_W_LOW_PIN, 0);
}

bool tmc6300_has_fault(void) {
    return !gpio_get(MOTOR_DIAG_PIN);
}

void tmc6300_set_pwm(uint16_t uh, uint16_t ul,
                     uint16_t vh, uint16_t vl,
                     uint16_t wh, uint16_t wl) {
    if (!tmc6300_initialized) {
        tmc6300_init();
    }

    tmc6300_write_level(MOTOR_PHASE_U_HIGH_PIN, uh);
    tmc6300_write_level(MOTOR_PHASE_U_LOW_PIN, ul);
    tmc6300_write_level(MOTOR_PHASE_V_HIGH_PIN, vh);
    tmc6300_write_level(MOTOR_PHASE_V_LOW_PIN, vl);
    tmc6300_write_level(MOTOR_PHASE_W_HIGH_PIN, wh);
    tmc6300_write_level(MOTOR_PHASE_W_LOW_PIN, wl);
}
