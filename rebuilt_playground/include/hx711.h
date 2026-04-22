#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <stdbool.h>

// Initialization and hardware checks
void hx711_init(void);
bool hx711_is_ready(void);

// Configuration
// Accepts 128 (Ch A), 64 (Ch A), or 32 (Ch B). Automatically handles dummy read.
void hx711_set_gain(uint8_t gain);

// Reading Functions
int32_t hx711_read(void);             // Returns raw, un-tared hardware value
void hx711_tare(uint8_t times);       // Averages 'times' readings to set the 0 offset
int32_t hx711_read_net(void);         // Returns raw value minus the tared offset

#endif // HX711_H