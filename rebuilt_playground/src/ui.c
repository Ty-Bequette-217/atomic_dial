#include "ui.h"

// Keep the arc variable hidden inside this file so nothing else can break it
static lv_obj_t * my_arc;

void ui_init(void) {
    // 1. Screen Background Object
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    // 2. Create the arc object
    my_arc = lv_arc_create(screen);
    lv_obj_set_size(my_arc, 200, 200);
    lv_obj_center(my_arc);
    lv_arc_set_bg_angles(my_arc, 0, 360); 
    lv_arc_set_range(my_arc, 0, 360);
    lv_arc_set_value(my_arc, 0);

    // 3. Styling the Arc to look like a Ball in a Track
    lv_obj_set_style_arc_color(my_arc, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_arc_width(my_arc, 10, LV_PART_MAIN);
    lv_obj_set_style_opa(my_arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(my_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(my_arc, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_pad_all(my_arc, 12, LV_PART_KNOB); 
}

void ui_update_arc(uint16_t value) {
    if (my_arc != NULL) {
        lv_arc_set_value(my_arc, value);
    }
}

void ui_set_state_visual(knob_state_t state) {
    if (my_arc == NULL) return;
    lv_color_t color;
    switch (state) {
        case KNOB_STATE_UNBOUNDED_NO_DETENTS:
            color = lv_color_hex(0x3B82F6);   // blue
            break;

        case KNOB_STATE_BOUNDED_0_10_NO_DETENTS:
            color = lv_color_hex(0x22C55E);   // green
            break;

        case KNOB_STATE_MULTI_REV_NO_DETENTS:
            color = lv_color_hex(0x06B6D4);   // cyan
            break;

        case KNOB_STATE_ON_OFF_STRONG_DETENT:
            color = lv_color_hex(0xEF4444);   // red
            break;

        case KNOB_STATE_RETURN_TO_CENTER:
            color = lv_color_hex(0xF59E0B);   // amber
            break;

        case KNOB_STATE_FINE_VALUES_NO_DETENTS:
            color = lv_color_hex(0x8B5CF6);   // violet
            break;

        case KNOB_STATE_FINE_VALUES_WITH_DETENTS:
            color = lv_color_hex(0xA855F7);   // purple
            break;

        case KNOB_STATE_COARSE_VALUES_STRONG_DETENTS:
            color = lv_color_hex(0xF97316);   // orange
            break;

        case KNOB_STATE_COARSE_VALUES_WEAK_DETENTS:
            color = lv_color_hex(0xFB7185);   // pink
            break;

        case KNOB_STATE_MAGNETIC_DETENTS:
            color = lv_color_hex(0x14B8A6);   // teal
            break;

        case KNOB_STATE_RETURN_TO_CENTER_WITH_DETENTS:
            color = lv_color_hex(0xEAB308);   // yellow
            break;

        case KNOB_STATE_COUNT:
        default:
            color = lv_color_hex(0x404040);   // fallback gray
            break;
    }
    lv_obj_set_style_arc_color(my_arc, color, LV_PART_MAIN);
}