#include <stdio.h>
#include "motor_control.h"

// Interrupt handler for motor fault detection
void motor_fault(uint gpio, uint32_t events) {
    // Clear the interrupt by acknowledging it
    gpio_acknowledge_irq(MOTOR_DIAG_PIN, events);
    printf("Motor fault detected\n");

    // Disable the motor driver to reset the motor
    disable_motor();
    sleep_ms(1000); // Delay in case fault not cleared immediately
    enable_motor(); // Re-enable
}

// Initialize the MOTOR_VIO pin (high by default, low to disable motor driver)
void init_motor_control(void) {
    // Initialize the GPIO pins
    gpio_init(MOTOR_VIO_PIN);
    gpio_init(MOTOR_DIAG_PIN);
    
    // Set IO
    gpio_set_dir(MOTOR_VIO_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_DIAG_PIN, GPIO_IN);
    
    // Set high by default (motor enabled)
    gpio_put(MOTOR_VIO_PIN, 1);
    
    // Enable pull-down (assumes fault signal pulls high)
    gpio_pull_down(MOTOR_DIAG_PIN);
    
    // Set interrupt to trigger on high level with callback
    gpio_set_irq_enabled_with_callback(MOTOR_DIAG_PIN, GPIO_IRQ_LEVEL_HIGH, true, &motor_fault);
}

// Enable the motor driver (set VIO high)
void enable_motor(void) {
    gpio_put(MOTOR_VIO_PIN, 1);
}

// Disable the motor driver (set VIO low)
void disable_motor(void) {
    gpio_put(MOTOR_VIO_PIN, 0);
}
