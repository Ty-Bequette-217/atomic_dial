#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H


#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"

// MOTOR DRIVER (all are pulled down)
#define MOTOR_PHASE_V_LOW_PIN       2
#define MOTOR_PHASE_V_HIGH_PIN      3
#define MOTOR_PHASE_W_LOW_PIN       4
#define MOTOR_PHASE_W_HIGH_PIN      5
#define MOTOR_PHASE_U_LOW_PIN       6
#define MOTOR_PHASE_U_HIGH_PIN      7
#define MOTOR_DIAG_PIN             14

// UART
#define DEBUG_UART         uart0
#define DEBUG_UART_RX_PIN  1
#define DEBUG_UART_TX_PIN  0
#define DEBUG_UART_BAUD    115200

#define SERIAL_UART        uart1
#define SERIAL_UART_RX_PIN 8
#define SERIAL_UART_TX_PIN 9

// STRAIN SENSOR (rate 1  80 samples / second)
#define STRAIN_DO_PIN  10
#define STRAIN_SCK_PIN 11

// LDR PINS
#define LDR_V_IN 45
#define LDR_ADC_CHAN 5
#define LDR_CLK_DIV 2.5e6

// LCD
#define LCD_SPI_CHAN spi0
#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 240
#define LCD_CMD_PIN 15
#define LCD_BACKLIGHT_PIN 16
#define LCD_BACKLIGHT_PWM_CHAN 0
#define LCD_CS_PIN 17
#define LCD_SCK_PIN 18
#define LCD_DATA_PIN 19
#define LCD_RST_PIN 20
#define BUTTON_PIN 21

// LED_DATA
#define LED_DATA_3V3_PIN  27; 

// AS5600 MAGNETIC ENCODER (I2C)
#define MAG_ENC_I2C_PORT  i2c0
#define MAG_ENC_I2C_SDA   28 
#define MAG_ENC_I2C_SCL   29 

// CONTROL SETTINGS
#define MOTOR_CONTROL_HZ    4000.0f
#define MOTOR_PWM_WRAP      1000

// motor config
#define MOTOR_DEFAULT_POLE_PAIRS    7
#define MOTOR_DEFAULT_DIRECTION     1
#define MOTOR_SUPPLY_VOLTAGE        5.0f
#define MOTOR_VOLTAGE_LIMIT         1.8f
#define MOTOR_DETENT_SPACING_DEG    15.0f
#define MOTOR_SOFT_DETENT_STRENGTH  0.45f
#define MOTOR_STRONG_DETENT_STRENGTH 1.20f
#define MOTOR_DETENT_DAMPING        0.004f
#define MOTOR_ENCODER_FILTER_ALPHA  0.18f

#endif
