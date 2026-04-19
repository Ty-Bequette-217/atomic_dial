#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#define MAX_DETENT_POSITIONS 5
#define SMARTKNOB_TEXT_LEN   51

typedef struct {
    int32_t position;
    float sub_position_unit;
    uint8_t position_nonce;

    int32_t min_position;
    int32_t max_position;   // max < min means unbounded

    float position_width_radians;
    float detent_strength_unit;
    float endstop_strength_unit;
    float snap_point;

    char text[SMARTKNOB_TEXT_LEN];

    uint32_t detent_positions_count;
    int32_t detent_positions[MAX_DETENT_POSITIONS];

    float snap_point_bias;
    int8_t led_hue;
} SmartKnobConfig;

typedef enum {
    KNOB_STATE_UNBOUNDED_NO_DETENTS = 0,
    KNOB_STATE_BOUNDED_0_10_NO_DETENTS,
    KNOB_STATE_MULTI_REV_NO_DETENTS,
    KNOB_STATE_ON_OFF_STRONG_DETENT,
    KNOB_STATE_RETURN_TO_CENTER,
    KNOB_STATE_FINE_VALUES_NO_DETENTS,
    KNOB_STATE_FINE_VALUES_WITH_DETENTS,
    KNOB_STATE_COARSE_VALUES_STRONG_DETENTS,
    KNOB_STATE_COARSE_VALUES_WEAK_DETENTS,
    KNOB_STATE_MAGNETIC_DETENTS,
    KNOB_STATE_RETURN_TO_CENTER_WITH_DETENTS,

    KNOB_STATE_COUNT
} knob_state_t;

typedef enum {
    KNOB_EVENT_NONE = 0,
    KNOB_EVENT_NEXT_MODE,
    KNOB_EVENT_PREV_MODE,
    KNOB_EVENT_BUTTON_PRESS,
    KNOB_EVENT_BUTTON_LONG_PRESS,
    KNOB_EVENT_HOME,
    KNOB_EVENT_RESET
} knob_event_t;

typedef struct {
    knob_state_t current_state;
    const SmartKnobConfig *active_config;
    bool config_dirty;
} SmartKnobStateMachine;

// Public API
void smartknob_sm_init(SmartKnobStateMachine *sm);
void smartknob_sm_handle_event(SmartKnobStateMachine *sm, knob_event_t event);
SmartKnobStateMachine *smartknob_sm_get_instance(void);
const SmartKnobConfig *smartknob_sm_get_config(const SmartKnobStateMachine *sm);
knob_state_t smartknob_sm_get_state(const SmartKnobStateMachine *sm);

#endif