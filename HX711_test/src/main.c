#include <stdio.h>
#include "pico/stdlib.h"
#include "board_config.h" 
#include "hx711.h"

// ==========================================
// ACTIVE MAIN: GAIN 64 CONFIGURATION
// ==========================================
int main(void) {
    stdio_init_all();
    
    sleep_ms(2000); 
    printf("Starting HX711 (Channel A / Gain 64) Test...\n");

    hx711_init();

    // Set the gain to 64 (this automatically handles the dummy read under the hood!)
    hx711_set_gain(64);

    float max_val = 8388608.0f;
    float e_volt = 3.3f;
    float R_right_upper = 1000.0f;
    
    // v_max math uses 64
    float v_max = (e_volt * 0.5f) / 64.0f; 

    while (1) {
        if (hx711_is_ready()) {
            
            int32_t data = hx711_read();

            float percent_v_max = (float)data / max_val;
            float volt_diff = percent_v_max * v_max;

            // The absolute physical resistance of the bottom-right leg
            float R_x = R_right_upper * (e_volt + (2.0f * volt_diff)) / (e_volt - (2.0f * volt_diff));

            printf("Raw Value: %d\r\n", data);
            printf("V difference: %f V\r\n", volt_diff);
            printf("Resistance of right leg bottom: %f Ohms\r\n", R_x);
            printf("----------------------------------------\r\n");
        }
        
        sleep_ms(500); 
    }

    return 0;
}


/* // ==========================================
// BACKUP MAIN: GAIN 128 CONFIGURATION
// ==========================================
int main(void) {
    stdio_init_all();
    
    sleep_ms(2000); 
    printf("Starting HX711 (Channel A / Gain 128) Test...\n");

    hx711_init();

    // Set the gain to 128
    hx711_set_gain(128);

    float max_val = 8388608.0f;
    float e_volt = 3.3f;
    float R_right_upper = 1000.0f;
    
    // v_max math uses 128
    float v_max = (e_volt * 0.5f) / 128.0f; 

    while (1) {
        if (hx711_is_ready()) {
            
            int32_t data = hx711_read();

            float percent_v_max = (float)data / max_val;
            float volt_diff = percent_v_max * v_max;

            float R_x = R_right_upper * (e_volt + (2.0f * volt_diff)) / (e_volt - (2.0f * volt_diff));

            printf("Raw Value: %d\r\n", data);
            printf("V difference: %f V\r\n", volt_diff);
            printf("Resistance of right leg bottom: %f Ohms\r\n", R_x);
            printf("----------------------------------------\r\n");
        }
        
        sleep_ms(500); 
    }

    return 0;
}
*/