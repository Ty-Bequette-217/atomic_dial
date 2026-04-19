#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <stdbool.h>

void hx711_init(void);
bool hx711_is_ready(void);

// --- NEW CONFIGURATION FUNCTION ---
// Accepts 128, 64, or 32. Automatically handles the dummy read.
void hx711_set_gain(uint8_t gain);

int32_t hx711_read(void);

#endif // HX711_H