#include "hx711.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "board_config.h"

// Default to 1 extra pulse (Channel A, Gain 128)
static uint8_t extra_pulses = 1; 

// Stores the baseline value calculated during tare()
static int32_t offset = 0; 

void hx711_init(void) {
    gpio_init(STRAIN_SCK_PIN);
    gpio_set_dir(STRAIN_SCK_PIN, GPIO_OUT);
    
    gpio_init(STRAIN_DO_PIN);
    gpio_set_dir(STRAIN_DO_PIN, GPIO_IN);

    gpio_put(STRAIN_SCK_PIN, 0);
}

void hx711_set_gain(uint8_t gain) {
    switch (gain) {
        case 128:
            extra_pulses = 1; // Channel A
            break;
        case 64:
            extra_pulses = 3; // Channel A
            break;
        case 32:
            extra_pulses = 2; // Channel B
            break;
        default:
            extra_pulses = 1; // Default to 128 if an invalid number is passed
            break;
    }
    
    // Force one dummy read right now so the hardware switches to the 
    // new gain setting before the user tries to take a real measurement
    hx711_read(); 
}

bool hx711_is_ready(void) {
    return gpio_get(STRAIN_DO_PIN) == 0;
}

int32_t hx711_read(void) {
    while (!hx711_is_ready()) {
        sleep_us(1);
    }

    int32_t value = 0;

    // Bit-bang 24 pulses to read the 24-bit data (MSB first)
    for (int i = 0; i < 24; i++) {
        gpio_put(STRAIN_SCK_PIN, 1);
        sleep_us(1); 
        value = (value << 1) | gpio_get(STRAIN_DO_PIN);
        gpio_put(STRAIN_SCK_PIN, 0);
        sleep_us(1);
    }

    // Pulse the dynamically configured number of extra times
    for (int i = 0; i < extra_pulses; i++) {
        gpio_put(STRAIN_SCK_PIN, 1);
        sleep_us(1);
        gpio_put(STRAIN_SCK_PIN, 0);
        sleep_us(1);
    }

    // --- The Two's Complement Trick ---
    if (value & 0x800000) {
        value |= 0xFF000000;
    }

    return value;
}

void hx711_tare(uint8_t times) {
    int64_t sum = 0;
    
    // Read the scale 'times' amount of times to get a stable average
    for (uint8_t i = 0; i < times; i++) {
        sum += hx711_read();
    }
    
    // Save the average as the new zero-point
    offset = (int32_t)(sum / times);
}

int32_t hx711_read_net(void) {
    // Return the difference between the current reading and the saved zero-point
    return hx711_read() - offset;
}