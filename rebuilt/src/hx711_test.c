#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// =========================
// HX711 pins
// =========================
#define HX711_DOUT_PIN 14
#define HX711_SCK_PIN  15

// =========================
// RP2350B QFN-80 analog debug pins
//
// Pick ADC-capable GPIOs that EXIST ON YOUR BOARD LAYOUT.
// Valid ADC GPIOs on RP2350B QFN-80 are:
//
// GPIO26 -> ADC0
// GPIO27 -> ADC1
// GPIO28 -> ADC2
// GPIO29 -> ADC3
// GPIO40 -> ADC4?  (see note below)
// GPIO41 -> ADC5?
// GPIO42 -> ADC6?
// GPIO43 -> ADC7?
// GPIO44 -> ADC4
// GPIO45 -> ADC5
// GPIO46 -> ADC6
// GPIO47 -> ADC7
//
// IMPORTANT:
// Use the ADC channel number that matches your SDK/device mapping.
// On RP2350 QFN-80, the datasheet exposes ADC inputs up to ADC7.
// =========================

// Example selection for QFN-80 board routing:
#define DEBUG_ADC_GPIO_APLUS   40
#define DEBUG_ADC_CH_APLUS     4

#define DEBUG_ADC_GPIO_AMINUS  41
#define DEBUG_ADC_CH_AMINUS    5

#define ADC_REF_VOLTAGE 3.3f
#define ADC_MAX_COUNTS  4095.0f

typedef enum {
    HX711_A_128 = 25,
    HX711_B_32  = 26,
    HX711_A_64  = 27
} hx711_mode_t;

static hx711_mode_t hx711_mode = HX711_A_128;
static int32_t hx711_offset = 0;

void hx711_init(void) {
    gpio_init(HX711_SCK_PIN);
    gpio_set_dir(HX711_SCK_PIN, GPIO_OUT);
    gpio_put(HX711_SCK_PIN, 0);

    gpio_init(HX711_DOUT_PIN);
    gpio_set_dir(HX711_DOUT_PIN, GPIO_IN);
}

bool hx711_is_ready(void) {
    return gpio_get(HX711_DOUT_PIN) == 0;
}

bool hx711_wait_ready_timeout_us(uint32_t timeout_us) {
    absolute_time_t deadline = make_timeout_time_us(timeout_us);

    while (!hx711_is_ready()) {
        if (absolute_time_diff_us(get_absolute_time(), deadline) <= 0) {
            return false;
        }
        tight_loop_contents();
    }
    return true;
}

bool hx711_read_once(int32_t *value, hx711_mode_t mode) {
    if (!hx711_wait_ready_timeout_us(1000000)) {
        return false;
    }

    int32_t count = 0;

    for (int i = 0; i < 24; i++) {
        gpio_put(HX711_SCK_PIN, 1);
        sleep_us(1);

        count <<= 1;
        if (gpio_get(HX711_DOUT_PIN)) {
            count |= 1;
        }

        gpio_put(HX711_SCK_PIN, 0);
        sleep_us(1);
    }

    for (int i = 24; i < mode; i++) {
        gpio_put(HX711_SCK_PIN, 1);
        sleep_us(1);
        gpio_put(HX711_SCK_PIN, 0);
        sleep_us(1);
    }

    if (count & 0x800000) {
        count |= ~0xFFFFFF;
    }

    *value = count;
    return true;
}

bool hx711_read_average(int32_t *avg_value, hx711_mode_t mode, int samples) {
    int64_t sum = 0;
    int32_t reading = 0;

    for (int i = 0; i < samples; i++) {
        if (!hx711_read_once(&reading, mode)) {
            return false;
        }
        sum += reading;
    }

    *avg_value = (int32_t)(sum / samples);
    return true;
}

bool hx711_tare(int samples) {
    int32_t avg = 0;
    if (!hx711_read_average(&avg, hx711_mode, samples)) {
        return false;
    }
    hx711_offset = avg;
    return true;
}

bool hx711_get_value(int32_t *value, int samples) {
    int32_t avg = 0;
    if (!hx711_read_average(&avg, hx711_mode, samples)) {
        return false;
    }
    *value = avg - hx711_offset;
    return true;
}

// -------------------------
// ADC debug
// -------------------------
void analog_debug_init(void) {
    adc_init();

    adc_gpio_init(DEBUG_ADC_GPIO_APLUS);
    adc_gpio_init(DEBUG_ADC_GPIO_AMINUS);
}

uint16_t read_adc_raw(uint input_channel) {
    adc_select_input(input_channel);
    sleep_us(5);
    return adc_read();
}

float adc_counts_to_volts(uint16_t counts) {
    return ((float)counts * ADC_REF_VOLTAGE) / ADC_MAX_COUNTS;
}

void read_bridge_debug(float *v_aplus, float *v_aminus, float *v_diff) {
    uint16_t raw_aplus  = read_adc_raw(DEBUG_ADC_CH_APLUS);
    uint16_t raw_aminus = read_adc_raw(DEBUG_ADC_CH_AMINUS);

    *v_aplus  = adc_counts_to_volts(raw_aplus);
    *v_aminus = adc_counts_to_volts(raw_aminus);
    *v_diff   = *v_aplus - *v_aminus;
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("RP2350B QFN-80 HX711 Debug Starting...\n");

    hx711_init();
    analog_debug_init();

    sleep_ms(500);

    if (hx711_tare(16)) {
        printf("Tare complete. Offset = %ld\n", hx711_offset);
    } else {
        printf("Tare failed. HX711 not responding.\n");
    }

    while (true) {
        int32_t hx_value = 0;
        float v_aplus = 0.0f, v_aminus = 0.0f, v_diff = 0.0f;

        bool hx_ok = hx711_get_value(&hx_value, 8);
        read_bridge_debug(&v_aplus, &v_aminus, &v_diff);

        printf("DOUT=%d | ", gpio_get(HX711_DOUT_PIN));

        if (hx_ok) {
            printf("HX711=%ld | ", hx_value);
        } else {
            printf("HX711=TIMEOUT | ");
        }

        printf("A+=%.5f V | A-=%.5f V | DIFF=%.6f V\n",
               v_aplus, v_aminus, v_diff);

        sleep_ms(100);
    }

    return 0;
}