#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "board_config.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/resets.h"
#include "hardware/dma.h"
#include "ldr_monitoring.h"

#define PWM_TOP 4095

volatile uint16_t ldr_adc_fifo_out = 0;


void init_adc() {
    adc_init(); 
    adc_gpio_init(LDR_V_IN); 
    adc_select_input(LDR_ADC_CHAN);
}

uint16_t read_adc() {
    return adc_read(); 
}

void init_adc_freerun() {
    adc_init(); 
    adc_gpio_init(LDR_V_IN); 
    adc_run(true);
    adc_set_clkdiv(LDR_CLK_DIV);
    adc_select_input(LDR_ADC_CHAN);    
}

void init_dma() {
    // config DMA Ch 0 to read from ADC FIFO and write to variable ldr_adc_fifo_out
    dma_hw->ch[0].read_addr = &adc_hw->fifo;
    dma_hw->ch[0].write_addr = &ldr_adc_fifo_out;
    dma_hw->ch[0].transfer_count = (1 << 28) | (1 << 0);
    dma_hw->ch[0].ctrl_trig = 0;
    uint32_t temp = 0;
    temp |= 1 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB;
    temp |= DREQ_ADC << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB;
    temp |= 0x1u << 0;
    dma_hw->ch[0].ctrl_trig = temp;
}

void init_adc_dma() {
    init_dma(); 
    init_adc_freerun(); 
    uint32_t mask = 1 << 0u; 
    mask |= 1 << 3u; 
    adc_hw->fcs |= mask;
}

void init_pwm_static(uint32_t period, uint32_t duty_cycle) {
    gpio_set_function(LCD_BACKLIGHT_PIN, GPIO_FUNC_PWM); // allocate pin function to PWM
    uint slice_num = pwm_gpio_to_slice_num(LCD_BACKLIGHT_PIN); // get PWM slice number
    pwm_set_clkdiv(slice_num, 150); // set PWM slice clock divider to 150
    uint32_t max_period = period - 1; // set wrapping counter to period - 1, writing to the register
    pwm_hw->slice[slice_num].top = max_period;
    uint16_t level = duty_cycle; // set cc to duty_cycle

    hw_write_masked(
    &pwm_hw->slice[slice_num].cc,
    ((uint)level) << (LCD_BACKLIGHT_PWM_CHAN ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB),
    LCD_BACKLIGHT_PWM_CHAN ? PWM_CH0_CC_B_BITS : PWM_CH0_CC_A_BITS);
    
    // Set PWM running (enabled)
    bool enabled = true;
    hw_write_masked(&pwm_hw->slice[slice_num].csr, bool_to_bit(enabled) << PWM_CH0_CSR_EN_LSB, PWM_CH0_CSR_EN_BITS);

}

void pwm_brightness_adjust() {
    // fill in
    // acknowledge the interrupt by writing a specific value to the appropriate PWM interrupt register
    uint slice_num = pwm_gpio_to_slice_num(LCD_BACKLIGHT_PIN);
    pwm_hw->intr = 1u << slice_num; 

    // set the duty cycle to the ratio of the LDR vs the known resistance for brightness
    // uint16_t pwm_brightness = (uint16_t)((ldr_adc_fifo_out * PWM_TOP) / 4095u);
    uint16_t adc_raw = ldr_adc_fifo_out & 0x0FFF;
    uint16_t pwm_brightness = (adc_raw * PWM_TOP) / 4095u;
    if (pwm_brightness > PWM_TOP) pwm_brightness = PWM_TOP;

    hw_write_masked(
    &pwm_hw->slice[slice_num].cc,
    ((uint)pwm_brightness) << (LCD_BACKLIGHT_PWM_CHAN ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB),
    LCD_BACKLIGHT_PWM_CHAN ? PWM_CH0_CC_B_BITS : PWM_CH0_CC_A_BITS);
}

void init_pwm_irq() {
    // in PWM registers
    // enable PWM interruptsfor PWM slice using WRAP0, don't need to do for 38 39
    uint slice_num = pwm_gpio_to_slice_num(LCD_BACKLIGHT_PIN);
    hw_set_bits(&pwm_hw->inte, 1u << slice_num);

    // set pwm_brightness_adjust as exclusive handler when it wraps to 0
    // irq number PWM_IRQ_WRAP_0 = 8
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_brightness_adjust);

    // enable interrupts for PWM
    irq_set_enabled(PWM_IRQ_WRAP, true);
    uint16_t current_period = pwm_hw->slice[slice_num].top;

    // set duty cycle to current_period
    pwm_hw->slice[slice_num].top = current_period;
}


