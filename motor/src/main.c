// Standard C Library Headers
#include <stdio.h>
#include <string.h>

// Raspberry Pi Pico SDK Headers
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/resets.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

// Third-Party Libraries
#include "lvgl.h"

// Project-Specific Local Headers
#include "board_config.h"
#include "as5600.h"
#include "hx711.h"
#include "ldr_monitoring.h"
#include "motor_feedback.h"
#include "display.h"
#include "ui/ui.h"

// --- GLOBALS & STATE ---
static volatile bool mode_advance_requested = false;
static bool strain_gauge_pressed = false;
static uint32_t last_strain_gauge_trigger_ms = 0;

static const int32_t STRAIN_GAUGE_PRESS_THRESHOLD = -300;
static const uint32_t STRAIN_GAUGE_DEBOUNCE_MS = 250;

static float wrap_degrees_360(float degrees) {
    while (degrees >= 360.0f) {
        degrees -= 360.0f;
    }
    while (degrees < 0.0f) {
        degrees += 360.0f;
    }
    return degrees;
}

static float wrap_degrees_delta(float degrees) {
    while (degrees > 180.0f) {
        degrees -= 360.0f;
    }
    while (degrees < -180.0f) {
        degrees += 360.0f;
    }
    return degrees;
}

static bool read_display_angle_degrees(ui_mode_t mode, float *degrees) {
    if (degrees == NULL) {
        return false;
    }

    if (mode == UI_MODE_UNBOUNDED_NO_DETENTS) {
        uint16_t raw_angle;
        if (!as5600_read_raw(&raw_angle)) {
            return false;
        }

        *degrees = (raw_angle * 360.0f) / 4095.0f;
        return true;
    }

    return motor_feedback_get_angle_degrees(degrees);
}

static bool mode_is_on_off_switch(ui_mode_t mode) {
    return mode == UI_MODE_ON_OFF_SWITCH;
}

// --- BUTTON LOGIC ---
static void button_gpio_isr(void) {
    if (gpio_get_irq_event_mask(BUTTON_PIN) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(BUTTON_PIN, GPIO_IRQ_EDGE_RISE);
        mode_advance_requested = true;
    }
}

static void button_init(void) {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    gpio_add_raw_irq_handler_masked(1u << BUTTON_PIN, button_gpio_isr);
    gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

// --- STRAIN GAUGE LOGIC ---
static void strain_gauge_init(void) {
    hx711_init();
    hx711_set_gain(64);

    printf("Keep hands off the strain gauge. Calibrating...\n");
    hx711_tare(20);
    printf("Strain gauge tare complete\n");
}

static void request_mode_advance_from_strain_gauge(void) {
    if (!hx711_is_ready()) {
        return;
    }

    int32_t strain_value = hx711_read_net();
    bool crossed_threshold = strain_value < STRAIN_GAUGE_PRESS_THRESHOLD;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (crossed_threshold && !strain_gauge_pressed &&
        (now - last_strain_gauge_trigger_ms > STRAIN_GAUGE_DEBOUNCE_MS)) {
        strain_gauge_pressed = true;
        last_strain_gauge_trigger_ms = now;
        mode_advance_requested = true;
        printf("Strain gauge mode advance. Value: %ld\n", strain_value);
    } else if (!crossed_threshold && strain_gauge_pressed &&
               strain_value > (STRAIN_GAUGE_PRESS_THRESHOLD / 2)) {
        strain_gauge_pressed = false;
    }
}

// --- MAIN LOOP ---
int main() {
    stdio_init_all();
    float filtered_angle = 0.0f;
    float angle_offset = 0.0f;
    const float angle_lpf_alpha = MOTOR_UI_FILTER_ALPHA;
    ui_mode_t current_mode = UI_MODE_UNBOUNDED_NO_DETENTS;
    absolute_time_t next_ui_update = make_timeout_time_ms(10);

    if (!as5600_init()) {
        printf("AS5600 init failed\n");
    }

    lcd_init();
    button_init();
    strain_gauge_init();

    // LDR controlling LCD backlight
    init_adc_dma();
    init_pwm_static(10000, 5000); 
    init_pwm_irq(); 
    
    // LVGL Setup
    lv_init();
    static lv_color_t buf_1[MY_DISP_HOR_RES * 24];
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, MY_DISP_HOR_RES * 24);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = my_disp_flush; 
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Call the external UI function!
    ui_init();
    ui_play_welcome_screen();
    ui_set_mode(current_mode);
    ui_update_arc(0);
    ui_set_center_value(0);

    if (!motor_feedback_init(current_mode)) {
        printf("Motor feedback disabled\n");
    }

    while (1) {
        motor_feedback_update();

        if (time_reached(next_ui_update)) {
            next_ui_update = make_timeout_time_ms(10);
            lv_timer_handler();
            request_mode_advance_from_strain_gauge();

            if (mode_advance_requested) {
                mode_advance_requested = false;
                current_mode = (ui_mode_t)((current_mode + 1) % UI_MODE_COUNT);
                motor_feedback_set_mode(current_mode);

                float motor_angle;
                if (read_display_angle_degrees(current_mode, &motor_angle)) {
                    angle_offset = motor_angle;
                } else {
                    uint16_t raw_angle;
                    if (as5600_read_raw(&raw_angle)) {
                        angle_offset = (raw_angle * 360.0f) / 4095.0f;
                    }
                }
                filtered_angle = 0.0f;
                ui_set_mode(current_mode);
                if (mode_is_on_off_switch(current_mode)) {
                    uint8_t switch_position = 0;
                    float switch_display_angle = MOTOR_SWITCH_OFF_ANGLE_DEG;
                    motor_feedback_get_switch_position(&switch_position);
                    motor_feedback_get_switch_display_degrees(&switch_display_angle);
                    ui_update_arc((uint16_t)switch_display_angle);
                    ui_set_center_text(switch_position ? "ON" : "OFF");
                } else {
                    ui_update_arc(0);
                    ui_set_center_value(0);
                }
            }

            if (mode_is_on_off_switch(current_mode)) {
                uint8_t switch_position = 0;
                float switch_display_angle = 0.0f;
                if (!motor_feedback_get_switch_position(&switch_position)
                        || !motor_feedback_get_switch_display_degrees(&switch_display_angle)) {
                    continue;
                }

                ui_update_arc((uint16_t)switch_display_angle);
                ui_set_center_text(switch_position ? "ON" : "OFF");
                continue;
            }

            float mapped_angle;
            if (!read_display_angle_degrees(current_mode, &mapped_angle)) {
                uint16_t raw_angle;
                if (as5600_read_raw(&raw_angle)) {
                    mapped_angle = (raw_angle * 360.0f) / 4095.0f;
                } else {
                    continue;
                }
            }

            float relative_angle = mapped_angle - angle_offset;
            relative_angle = wrap_degrees_360(relative_angle);
            filtered_angle = wrap_degrees_360(
                filtered_angle + angle_lpf_alpha * wrap_degrees_delta(relative_angle - filtered_angle)
            );
            ui_update_arc((uint16_t)filtered_angle);
            ui_set_center_value((uint16_t)filtered_angle);
        }
    }

    return 0;
}
