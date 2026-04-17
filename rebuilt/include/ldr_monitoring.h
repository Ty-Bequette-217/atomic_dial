#ifndef LDR_MONITORING_H
#define LDR_MONITORING_H

#include <stdint.h>

extern volatile uint16_t ldr_adc_fifo_out;

void init_adc(void);
uint16_t read_adc(void);
void init_adc_freerun(void);
void init_dma(void);
void init_adc_dma(void);
void init_pwm_static(uint32_t period, uint32_t duty_cycle);
void pwm_brightness_adjust(void);
void init_pwm_irq(void);

#endif