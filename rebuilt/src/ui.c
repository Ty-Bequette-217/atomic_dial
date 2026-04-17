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