#include <stdio.h>

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
#include "ldr_monitoring.h"
#include "mode_controls.h"
#include "motor_modes.h"
#include "motor_feedback.h"
#include "display.h"
#include "ui/ui.h"

// --- MAIN LOOP ---
int main() {
    stdio_init_all();
    ui_mode_t current_mode = UI_MODE_UNBOUNDED_NO_DETENTS;
    absolute_time_t next_ui_update = make_timeout_time_ms(10);

    if (!as5600_init()) {
        printf("AS5600 init failed\n");
    }

    lcd_init();
    mode_controls_init();

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
    ui_show_startup_screen("Calibrating...");
    lv_timer_handler();

    if (!motor_feedback_init(current_mode)) {
        printf("Motor feedback disabled\n");
    }

    motor_modes_init(current_mode);

    while (1) {
        motor_feedback_update();

        if (time_reached(next_ui_update)) {
            next_ui_update = make_timeout_time_ms(10);
            lv_timer_handler();
            mode_controls_update();

            if (mode_controls_consume_advance_request()) {
                current_mode = (ui_mode_t)((current_mode + 1) % UI_MODE_COUNT);
                motor_feedback_set_mode(current_mode);
                motor_modes_set_mode(current_mode);
            }

            motor_modes_update(current_mode);
        }
    }

    return 0;
}
