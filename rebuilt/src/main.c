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
#include <stdint.h>
#include "lvgl.h"
#include "ui.h"

int main(void) {
    stdio_init_all();

    // LDR controlling LCD backlight
    init_adc_dma();
    init_pwm_static(10000, 5000); // Start out with 500/1000, 50% brightness
    init_pwm_irq(); // Initialize PWM IRQ for variable duty cycle

    for(;;){

        // LDR Display Brightness Readout
        printf("ADC Result: %d     \r", adc_hw->result);
        fflush(stdout);
        sleep_ms(250);
    }
}