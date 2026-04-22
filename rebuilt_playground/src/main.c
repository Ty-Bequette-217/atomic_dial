// Standard C Library Headers
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Raspberry Pi Pico SDK Headers
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/resets.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

// Third-Party Libraries
#include "lvgl.h"

// Project-Specific Local Headers
#include "board_config.h"
#include "display.h"
#include "hx711.h"
#include "ldr_monitoring.h"
#include "motor_task.h"
#include "state_machine.h"
#include "ui.h"

int main(void) {
    stdio_init_all();

    // Motor global vars
    SmartKnobStateMachine *sm;
    motor_task_t motor;

    // init motor
    smartknob_sm_init(NULL);
    sm = smartknob_sm_get_instance();

    motor_task_init(&motor);
    motor_task_set_config(&motor, smartknob_sm_get_config(sm));


    // LVGL Setup
    lcd_init();
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
    ui_set_state_visual(smartknob_sm_get_state(sm));

    // LDR controlling LCD backlight
    init_adc_dma();
    init_pwm_static(10000, 5000); // Start out with 500/1000, 50% brightness
    init_pwm_irq(); // Initialize PWM IRQ for variable duty cycle

    // HX711 Setup
    hx711_init();
    hx711_set_gain(64);

    printf("Please keep hands off the FSR... Calibrating...\n");
    hx711_tare(20);
    printf("Tare complete!\n");

    // Software Button Variables
    // Tune this threshold! (More negative = requires harder press)
    int32_t fsr_press_threshold = -30000; 
    bool fsr_is_pressed = false;
    uint32_t last_fsr_trigger_ms = 0;
    
    while(1){
        // FSR button logic
        if (hx711_is_ready()) {
            int32_t hx_val = hx711_read_net(); 
            
            bool crossed_threshold = (hx_val < fsr_press_threshold);
            uint32_t now = to_ms_since_boot(get_absolute_time());

            // Edge Detection: Trigger only on the initial press, with 250ms debounce
            if (crossed_threshold && !fsr_is_pressed && (now - last_fsr_trigger_ms > 250)) {
                fsr_is_pressed = true; 
                last_fsr_trigger_ms = now;

                printf("FSR Clicked! Value: %d\n", hx_val);

                // FIRE THE EVENT: Acts exactly like the physical push-button!
                smartknob_sm_handle_event(sm, KNOB_EVENT_NEXT_MODE);
            } 
            // Release Detection: Unlock when finger is lifted (Hysteresis applied)
            else if (!crossed_threshold && fsr_is_pressed) {
                if (hx_val > (fsr_press_threshold / 2)) {
                    fsr_is_pressed = false;
                }
            }
        }
        lv_timer_handler();
        // Map the 12-bit ADC (0-4095) to the 360-degree Arc
        uint16_t mapped_val = (adc_hw->result * 360) / 4095;
        ui_update_arc(mapped_val);

        // // LDR Display Brightness Readout
        // printf("ADC Result: %d     \r", adc_hw->result);
        // fflush(stdout);

        // MOTOR CONFIG CHECK
        if (sm->config_dirty) {
            motor_task_set_config(&motor, smartknob_sm_get_config(sm));
            sm->config_dirty = false;
        }
        motor_task_update(&motor);

        sleep_ms(5);
    }
}
