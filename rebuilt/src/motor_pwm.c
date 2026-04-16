#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "board_config.h"
#include "motor_pwm.h"

static uint slice_ul;
static uint slice_uh;
static uint slice_vl;
static uint slice_vh;
static uint slice_wl;
static uint slice_wh;

static inline float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

static void setup_pwm_pin(uint pin, uint *slice_out) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    *slice_out = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(*slice_out, MOTOR_PWM_WRAP);
    pwm_set_enabled(*slice_out, true);
}

void motor_pwm_init(motor_pwm_t *m) {
    setup_pwm_pin(MOTOR_PHASE_U_LOW_PIN,  &slice_ul);
    setup_pwm_pin(MOTOR_PHASE_U_HIGH_PIN, &slice_uh);
    setup_pwm_pin(MOTOR_PHASE_V_LOW_PIN,  &slice_vl);
    setup_pwm_pin(MOTOR_PHASE_V_HIGH_PIN, &slice_vh);
    setup_pwm_pin(MOTOR_PHASE_W_LOW_PIN,  &slice_wl);
    setup_pwm_pin(MOTOR_PHASE_W_HIGH_PIN, &slice_wh);

    gpio_init(MOTOR_DIAG_PIN);
    gpio_set_dir(MOTOR_DIAG_PIN, GPIO_IN);
    gpio_pull_down(MOTOR_DIAG_PIN);

    m->duty_u = 0.5f;
    m->duty_v = 0.5f;
    m->duty_w = 0.5f;
    m->enabled = false;

    motor_pwm_enable(m, true);
    motor_pwm_set_duty_3phase(m, 0.5f, 0.5f, 0.5f);
}

void motor_pwm_enable(motor_pwm_t *m, bool en) {
    m->enabled = en;

    if (!en) {
        pwm_set_gpio_level(MOTOR_PHASE_U_HIGH_PIN, 0);
        pwm_set_gpio_level(MOTOR_PHASE_U_LOW_PIN, 0);
        pwm_set_gpio_level(MOTOR_PHASE_V_HIGH_PIN, 0);
        pwm_set_gpio_level(MOTOR_PHASE_V_LOW_PIN, 0);
        pwm_set_gpio_level(MOTOR_PHASE_W_HIGH_PIN, 0);
        pwm_set_gpio_level(MOTOR_PHASE_W_LOW_PIN, 0);
    }
}

void motor_pwm_set_duty_3phase(motor_pwm_t *m, float du, float dv, float dw) {
    du = clamp01(du);
    dv = clamp01(dv);
    dw = clamp01(dw);

    m->duty_u = du;
    m->duty_v = dv;
    m->duty_w = dw;

    if (!m->enabled) {
        return;
    }

    uint16_t level_u = (uint16_t)(du * MOTOR_PWM_WRAP);
    uint16_t level_v = (uint16_t)(dv * MOTOR_PWM_WRAP);
    uint16_t level_w = (uint16_t)(dw * MOTOR_PWM_WRAP);

    // For first bring-up, mirror low/high sides equally.
    // Later this should be replaced with proper complementary PWM + deadtime.
    pwm_set_gpio_level(MOTOR_PHASE_U_HIGH_PIN, level_u);
    pwm_set_gpio_level(MOTOR_PHASE_U_LOW_PIN,  level_u);

    pwm_set_gpio_level(MOTOR_PHASE_V_HIGH_PIN, level_v);
    pwm_set_gpio_level(MOTOR_PHASE_V_LOW_PIN,  level_v);

    pwm_set_gpio_level(MOTOR_PHASE_W_HIGH_PIN, level_w);
    pwm_set_gpio_level(MOTOR_PHASE_W_LOW_PIN,  level_w);
}