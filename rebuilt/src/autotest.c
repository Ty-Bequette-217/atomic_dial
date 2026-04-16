#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "board_config.h"
#include "motor_pwm.h"
#include "motor_control.h"
#include "encoder_mt6701.h"
#include "autotest.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void core1_blink(void) {
    const uint pin = LED_DATA_3V3_PIN;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);

    while (true) {
        gpio_xor_mask(1u << pin);
        sleep_ms(250);
    }
}

static void test_heartbeat(void) {
    const uint pin = LCD_BACKLIGHT_PIN;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);

    printf("TEST_HEARTBEAT start\n");
    multicore_launch_core1(core1_blink);

    for (int i = 0; i < 20; i++) {
        gpio_xor_mask(1u << pin);
        printf("AT,HEARTBEAT,CORE0_TICK=%d\n", i);
        sleep_ms(500);
    }
}

static void test_uart(void) {
    printf("TEST_UART start\n");
    for (int i = 0; i < 20; i++) {
        printf("AT,UART,SEQ=%d\n", i);
        sleep_ms(100);
    }
}

static void test_pwm_static(void) {
    motor_pwm_t pwm;
    motor_pwm_init(&pwm);

    printf("TEST_PWM_STATIC start\n");

    motor_pwm_set_duty_3phase(&pwm, 0.25f, 0.25f, 0.25f);
    printf("AT,PWM,DU=0.25,DV=0.25,DW=0.25\n");
    sleep_ms(1000);

    motor_pwm_set_duty_3phase(&pwm, 0.50f, 0.50f, 0.50f);
    printf("AT,PWM,DU=0.50,DV=0.50,DW=0.50\n");
    sleep_ms(1000);

    motor_pwm_set_duty_3phase(&pwm, 0.75f, 0.75f, 0.75f);
    printf("AT,PWM,DU=0.75,DV=0.75,DW=0.75\n");
    sleep_ms(1000);
}

static void test_pwm_sweep(void) {
    motor_pwm_t pwm;
    motor_control_t mc;

    motor_pwm_init(&pwm);
    motor_control_init(&mc, &pwm);

    printf("TEST_PWM_SWEEP start\n");

    for (int i = 0; i < 720; i++) {
        float mech = (2.0f * (float)M_PI * (float)i) / 720.0f;
        motor_control_update(&mc, mech, 1.0f, 0.0f);

        if ((i % 60) == 0) {
            printf("AT,SWEEP,MECH=%.3f,ELEC=%.3f\n",
                   mech, mc.electrical_angle_rad);
        }
        sleep_ms(10);
    }
}

static void test_spi_master(void) {
    printf("TEST_SPI_MASTER start\n");

    spi_init(MAG_SPI, 1000 * 1000);
    gpio_set_function(MAG_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MAG_DO_PIN, GPIO_FUNC_SPI);

    gpio_init(MAG_CSN_PIN);
    gpio_set_dir(MAG_CSN_PIN, GPIO_OUT);
    gpio_put(MAG_CSN_PIN, 1);

    for (int i = 0; i < 100; i++) {
        uint8_t tx[2] = {0x00, 0x00};
        uint8_t rx[2] = {0x00, 0x00};

        gpio_put(MAG_CSN_PIN, 0);
        spi_write_read_blocking(MAG_SPI, tx, rx, 2);
        gpio_put(MAG_CSN_PIN, 1);

        printf("AT,SPI,SEQ=%d,RX=%02X%02X\n", i, rx[0], rx[1]);
        sleep_ms(10);
    }
}

static void test_diag_input(void) {
    gpio_init(MOTOR_DIAG_PIN);
    gpio_set_dir(MOTOR_DIAG_PIN, GPIO_IN);
    gpio_pull_down(MOTOR_DIAG_PIN);

    printf("TEST_GPIO_INPUT start\n");
    printf("Drive MOTOR_DIAG_PIN from AD3 pattern generator now\n");

    int last = gpio_get(MOTOR_DIAG_PIN);

    for (int i = 0; i < 500; i++) {
        int now = gpio_get(MOTOR_DIAG_PIN);
        if (now != last) {
            printf("AT,DIAG,EDGE=%d,T_MS=%lld\n",
                   now, to_ms_since_boot(get_absolute_time()));
            last = now;
        }
        sleep_ms(10);
    }
}

void autotest_run(void) {
    sleep_ms(1000);
    printf("AUTOTEST begin\n");

    test_heartbeat();
    test_uart();
    test_pwm_static();
    test_pwm_sweep();
    test_spi_master();
    test_diag_input();

    printf("AUTOTEST done\n");
}