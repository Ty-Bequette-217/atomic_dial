#include "hx711.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "board_config.h"

static uint8_t extra_pulses = 1; 
static int32_t offset = 0; 
static int32_t last_raw_value = 0;

void hx711_init(void) {
    gpio_init(STRAIN_SCK_PIN);
    gpio_set_dir(STRAIN_SCK_PIN, GPIO_OUT);
    gpio_init(STRAIN_DO_PIN);
    gpio_set_dir(STRAIN_DO_PIN, GPIO_IN);
    gpio_put(STRAIN_SCK_PIN, 0);
}

void hx711_set_gain(uint8_t gain) {
    if (gain == 128) extra_pulses = 1;
    else if (gain == 64) extra_pulses = 3;
    else if (gain == 32) extra_pulses = 2;
    else extra_pulses = 1;
    hx711_read(); 
}

bool hx711_is_ready(void) {
    return gpio_get(STRAIN_DO_PIN) == 0;
}

int32_t hx711_read(void) {
    if (!hx711_is_ready()) return last_raw_value; // Return last value instead of stalling

    int32_t value = 0;
    for (int i = 0; i < 24; i++) {
        gpio_put(STRAIN_SCK_PIN, 1);
        sleep_us(1); 
        value = (value << 1) | gpio_get(STRAIN_DO_PIN);
        gpio_put(STRAIN_SCK_PIN, 0);
        sleep_us(1);
    }
    for (int i = 0; i < extra_pulses; i++) {
        gpio_put(STRAIN_SCK_PIN, 1); sleep_us(1);
        gpio_put(STRAIN_SCK_PIN, 0); sleep_us(1);
    }
    if (value & 0x800000) value |= 0xFF000000;
    last_raw_value = value;
    return value;
}

void hx711_tare(uint8_t times) {
    int64_t sum = 0;
    for (uint8_t i = 0; i < times; i++) sum += hx711_read();
    offset = (int32_t)(sum / times);
}

int32_t hx711_read_net(void) {
    return hx711_read() - offset;
}