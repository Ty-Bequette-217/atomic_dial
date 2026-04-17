#include "state_machine.h"
#include <string.h>
#include "board_config.h"

#define DEBOUNCE_MS         200

static SmartKnobStateMachine knob_sm;
static volatile uint32_t last_button_time_ms = 0;

static const SmartKnobConfig configs[KNOB_STATE_COUNT] = {
    [KNOB_STATE_UNBOUNDED_NO_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 0,
        .min_position = 0,
        .max_position = -1, // max < min => unbounded
        .position_width_radians = 10.0f * PI / 180.0f,
        .detent_strength_unit = 0.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Unbounded\nNo detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 200,
    },

    [KNOB_STATE_BOUNDED_0_10_NO_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 1,
        .min_position = 0,
        .max_position = 10,
        .position_width_radians = 10.0f * PI / 180.0f,
        .detent_strength_unit = 0.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Bounded 0-10\nNo detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 0,
    },

    [KNOB_STATE_MULTI_REV_NO_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 2,
        .min_position = 0,
        .max_position = 72,
        .position_width_radians = 10.0f * PI / 180.0f,
        .detent_strength_unit = 0.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Multi-rev\nNo detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 73,
    },

    [KNOB_STATE_ON_OFF_STRONG_DETENT] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 3,
        .min_position = 0,
        .max_position = 1,
        .position_width_radians = 60.0f * PI / 180.0f,
        .detent_strength_unit = 1.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 0.55f,
        .text = "On/off\nStrong detent",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 157,
    },

    [KNOB_STATE_RETURN_TO_CENTER] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 4,
        .min_position = 0,
        .max_position = 0,
        .position_width_radians = 60.0f * PI / 180.0f,
        .detent_strength_unit = 0.01f,
        .endstop_strength_unit = 0.6f,
        .snap_point = 1.1f,
        .text = "Return-to-center",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 45,
    },

    [KNOB_STATE_FINE_VALUES_NO_DETENTS] = {
        .position = 127,
        .sub_position_unit = 0.0f,
        .position_nonce = 5,
        .min_position = 0,
        .max_position = 255,
        .position_width_radians = 1.0f * PI / 180.0f,
        .detent_strength_unit = 0.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Fine values\nNo detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 219,
    },

    [KNOB_STATE_FINE_VALUES_WITH_DETENTS] = {
        .position = 127,
        .sub_position_unit = 0.0f,
        .position_nonce = 5,
        .min_position = 0,
        .max_position = 255,
        .position_width_radians = 1.0f * PI / 180.0f,
        .detent_strength_unit = 1.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Fine values\nWith detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 25,
    },

    [KNOB_STATE_COARSE_VALUES_STRONG_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 6,
        .min_position = 0,
        .max_position = 31,
        .position_width_radians = 8.225806452f * PI / 180.0f,
        .detent_strength_unit = 2.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Coarse values\nStrong detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 200,
    },

    [KNOB_STATE_COARSE_VALUES_WEAK_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 6,
        .min_position = 0,
        .max_position = 31,
        .position_width_radians = 8.225806452f * PI / 180.0f,
        .detent_strength_unit = 0.2f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Coarse values\nWeak detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.0f,
        .led_hue = 0,
    },

    [KNOB_STATE_MAGNETIC_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 7,
        .min_position = 0,
        .max_position = 31,
        .position_width_radians = 7.0f * PI / 180.0f,
        .detent_strength_unit = 2.5f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 0.7f,
        .text = "Magnetic detents",
        .detent_positions_count = 4,
        .detent_positions = {2, 10, 21, 22, 0},
        .snap_point_bias = 0.0f,
        .led_hue = 73,
    },

    [KNOB_STATE_RETURN_TO_CENTER_WITH_DETENTS] = {
        .position = 0,
        .sub_position_unit = 0.0f,
        .position_nonce = 8,
        .min_position = -6,
        .max_position = 6,
        .position_width_radians = 60.0f * PI / 180.0f,
        .detent_strength_unit = 1.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 0.55f,
        .text = "Return-to-center\nwith detents",
        .detent_positions_count = 0,
        .detent_positions = {0},
        .snap_point_bias = 0.4f,
        .led_hue = 157,
    }
};

static void smartknob_apply_config(SmartKnobStateMachine *sm, knob_state_t new_state) {
    sm->current_state = new_state;
    sm->active_config = &configs[new_state];
    sm->config_dirty = true;

    // Put hardware update hooks here:
    // - send config to motor control loop
    // - update LED hue
    // - update screen text
    // - reset/set position if needed
}

void smartknob_sm_init(SmartKnobStateMachine *sm) {
    if (!sm) return;
    sm->current_state = KNOB_STATE_UNBOUNDED_NO_DETENTS;
    sm->active_config = &configs[KNOB_STATE_UNBOUNDED_NO_DETENTS];
    sm->config_dirty = true;
}

static knob_state_t next_state(knob_state_t s) {
    return (knob_state_t)((s + 1) % KNOB_STATE_COUNT);
}

static knob_state_t prev_state(knob_state_t s) {
    return (s == 0) ? (KNOB_STATE_COUNT - 1) : (knob_state_t)(s - 1);
}

void smartknob_sm_handle_event(SmartKnobStateMachine *sm, knob_event_t event) {
    if (!sm) return;

    knob_state_t new_state = sm->current_state;

    switch (event) {
        case KNOB_EVENT_NEXT_MODE:
            new_state = next_state(sm->current_state);
            break;

        case KNOB_EVENT_PREV_MODE:
            new_state = prev_state(sm->current_state);
            break;

        case KNOB_EVENT_HOME:
        case KNOB_EVENT_RESET:
            new_state = KNOB_STATE_UNBOUNDED_NO_DETENTS;
            break;

        case KNOB_EVENT_BUTTON_PRESS:
            // Example behavior:
            // jump between a small subset of common modes if desired
            // new_state = KNOB_STATE_FINE_VALUES_WITH_DETENTS;
            break;

        case KNOB_EVENT_BUTTON_LONG_PRESS:
            // Example behavior:
            // new_state = KNOB_STATE_RETURN_TO_CENTER;
            break;

        case KNOB_EVENT_NONE:
        default:
            break;
    }

    if (new_state != sm->current_state) {
        smartknob_apply_config(sm, new_state);
    }
}

const SmartKnobConfig *smartknob_sm_get_config(const SmartKnobStateMachine *sm) {
    return sm ? sm->active_config : 0;
}

knob_state_t smartknob_sm_get_state(const SmartKnobStateMachine *sm) {
    return sm ? sm->current_state : KNOB_STATE_UNBOUNDED_NO_DETENTS;
}

static void apply_current_config(SmartKnobStateMachine *sm) {
    if (!sm || !sm->config_dirty) return;

    const SmartKnobConfig *cfg = smartknob_sm_get_config(sm);

    printf("\n=== ACTIVE STATE ===\n");
    printf("state enum: %d\n", smartknob_sm_get_state(sm));
    printf("text: %s\n", cfg->text);
    printf("position: %ld\n", (long)cfg->position);
    printf("min: %ld\n", (long)cfg->min_position);
    printf("max: %ld\n", (long)cfg->max_position);
    printf("width rad: %.4f\n", cfg->position_width_radians);
    printf("detent strength: %.3f\n", cfg->detent_strength_unit);
    printf("endstop strength: %.3f\n", cfg->endstop_strength_unit);
    printf("snap point: %.3f\n", cfg->snap_point);
    printf("snap bias: %.3f\n", cfg->snap_point_bias);
    printf("led hue: %d\n", cfg->led_hue);
    printf("====================\n");

    // This is where you'd really push config into:
    // - motor controller
    // - LED driver
    // - display task
    // - core1 mailbox/FIFO if needed

    sm->config_dirty = false;
}

static void button_irq_handler(uint gpio, uint32_t events) {
    if (gpio != MODE_BUTTON_PIN) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Simple debounce
    if ((now - last_button_time_ms) < DEBOUNCE_MS) {
        return;
    }

    last_button_time_ms = now;

    // Active-low button: only advance if the pin is actually low
    if (!gpio_get(MODE_BUTTON_PIN)) {
        smartknob_sm_handle_event(&knob_sm, KNOB_EVENT_NEXT_MODE);
    }
}

static void button_init(void) {
    gpio_init(MODE_BUTTON_PIN);
    gpio_set_dir(MODE_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(MODE_BUTTON_PIN);

    // Trigger on falling edge (button press to GND)
    gpio_set_irq_enabled_with_callback(
        MODE_BUTTON_PIN,
        GPIO_IRQ_EDGE_FALL,
        true,
        &button_irq_handler
    );
}
