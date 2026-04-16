#include <stdio.h>
#include "pico/stdlib.h"

#include "board_config.h"
#include "encoder_mt6701.h"
#include "motor_pwm.h"
#include "motor_control.h"

void core1_main(void) {
    encoder_mt6701_t encoder;
    motor_pwm_t motor_pwm;
    motor_control_t motor;

    encoder_mt6701_init(&encoder);
    motor_pwm_init(&motor_pwm);
    motor_control_init(&motor, &motor_pwm);

    const float dt = 1.0f / MOTOR_CONTROL_HZ;
    absolute_time_t next = get_absolute_time();

    float torque_cmd = 0.30f;

    printf("core1 started\n");

    while (true) {
        float mech_angle = encoder_mt6701_read_angle_rad(&encoder);
        float mech_vel = encoder_mt6701_read_velocity_rad_s(&encoder, dt);

        motor_control_update(&motor, mech_angle, torque_cmd, dt);

        static uint32_t print_div = 0;
        print_div++;
        if (print_div >= 400) {
            print_div = 0;
            printf("angle=%.3f rad, vel=%.3f rad/s, elec=%.3f rad\n",
                   mech_angle, mech_vel, motor.electrical_angle_rad);
        }

        next = delayed_by_us(next, (uint64_t)(1000000.0f / MOTOR_CONTROL_HZ));
        sleep_until(next);
    }
}