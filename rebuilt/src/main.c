#include <stdio.h>
#include "pico/stdlib.h"
#include "board_config.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/resets.h"
#include "hardware/dma.h"
#include "ldr_monitoring.h"
#include "state_machine.h"
#include "motor_task.h"
#include <stdint.h>
#include "lvgl.h"
#include "ui.h"
#include "display.h"

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
    
    while(1){
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
