#include "mode_controls.h"
#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "board_config.h"
#include "hx711.h"
#include "motor_feedback.h"

static volatile bool mode_advance_requested = false;
static bool strain_gauge_pressed = false;

static volatile bool button_press_latched = false;
static volatile uint32_t last_button_trigger_us = 0;
static uint32_t button_release_candidate_us = 0;

static const uint32_t BUTTON_DEBOUNCE_US = 50000;
static const uint32_t BUTTON_RELEASE_DEBOUNCE_US = 20000;

static void button_gpio_isr(void) {
    if (gpio_get_irq_event_mask(BUTTON_PIN) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(BUTTON_PIN, GPIO_IRQ_EDGE_RISE);
        const uint32_t now_us = time_us_32();
        if (button_press_latched || (uint32_t)(now_us - last_button_trigger_us) < BUTTON_DEBOUNCE_US) return;
        button_press_latched = true;
        last_button_trigger_us = now_us;
        mode_advance_requested = true;
    }
}

static void button_init(void) {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);
    gpio_add_raw_irq_handler_masked(1u << BUTTON_PIN, button_gpio_isr);
    gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

void mode_controls_init(void) {
    button_init();
    hx711_init();
    hx711_set_gain(64);
    hx711_tare(20);
}

void mode_controls_update(void) {
    if (button_press_latched && !gpio_get(BUTTON_PIN)) {
        const uint32_t now_us = time_us_32();
        if (button_release_candidate_us == 0) button_release_candidate_us = now_us;
        if ((uint32_t)(now_us - button_release_candidate_us) >= BUTTON_RELEASE_DEBOUNCE_US) {
            button_press_latched = false;
            button_release_candidate_us = 0;
        }
    } else { button_release_candidate_us = 0; }

    if (hx711_is_ready()) {
        const int32_t strain_value = hx711_read_net();
        if (strain_value < -300 && !strain_gauge_pressed) {
            strain_gauge_pressed = true;
            mode_advance_requested = true;
            motor_feedback_play_click(true);
        } else if (strain_value > -150 && strain_gauge_pressed) {
            strain_gauge_pressed = false;
            motor_feedback_play_click(false);
        }
    }
}

bool mode_controls_consume_advance_request(void) {
    if (!mode_advance_requested) return false;
    mode_advance_requested = false;
    return true;
}