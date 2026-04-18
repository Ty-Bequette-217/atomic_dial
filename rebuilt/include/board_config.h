#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"

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

// // AMBIENT LIGHT SENSOR I2C (all pulled up)
// #define I2C_SDA_PIN 12
// #define I2C_SCL_PIN 13

// LDR PINS (ADC)
#define LDR_V_IN 45
#define LDR_ADC_CHAN 5

// LCD (SPI)
#define LCD_SPI           spi0
#define LCD_HOR_RES 240
#define LCD_DISP_VER_RES 240
#define LCD_CMD_PIN       15
#define LCD_BACKLIGHT_PIN 16
#define LCD_BACKLIGHT_PWM_CHAN 0
#define LCD_CS_PIN        17
#define LCD_SCK_PIN       18
#define LCD_DATA_PIN      19
#define LCD_RST_PIN       20

// MODE CHANGE BUTTON (GPIO)
#define MODE_BUTTON_PIN 21

// LED_DATA
#define LED_DATA_3V3_PIN  27;

// AS5600 MAGNETIC ENCODER (I2C)
#define MAG_ENC_I2C_PORT  i2c0
#define MAG_ENC_I2C_SDA   28 
#define MAG_ENC_I2C_SCL   29 
// #define MAG_CLK_PIN  30 // already pulled up

// CONTROL SETTINGS
#define MOTOR_CONTROL_HZ    4000.0f
#define MOTOR_PWM_WRAP      1000

// encoder config
#define ENCODER_COUNTS_PER_REV      16384.0f

// motor config
#define MOTOR_DEFAULT_POLE_PAIRS    7
#define MOTOR_DEFAULT_DIRECTION     1

#endif