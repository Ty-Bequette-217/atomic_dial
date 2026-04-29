#include "ui.h"
#include <stdio.h>
#include "pico/stdlib.h"

// Keep the arc variable hidden inside this file so nothing else can break it
static lv_obj_t * my_arc;
static lv_obj_t * text_container;
static lv_obj_t * value_label;
static lv_obj_t * mode_label;
static lv_obj_t * detent_label;
static lv_obj_t * top_marker_line;

enum {
    UI_ARC_SIZE = 200,
    UI_ARC_WIDTH = 10,
    UI_KNOB_PAD = 6,
    UI_KNOB_DIAMETER = UI_ARC_WIDTH + (UI_KNOB_PAD * 2),
    UI_ARC_CENTER = UI_ARC_SIZE / 2,
};

static lv_color_t ui_get_knob_color(ui_mode_t mode) {
    switch (mode) {
        case UI_MODE_SOFT_DETENTS:
            return lv_color_hex(0x00C853);
        case UI_MODE_STRONG_DETENTS:
            return lv_color_hex(0xD50000);
        case UI_MODE_ON_OFF_SWITCH:
            return lv_color_hex(0xFFB300);
        case UI_MODE_UNBOUNDED_NO_DETENTS:
        case UI_MODE_COUNT:
        default:
            return lv_color_hex(0xFFFFFF);
    }
}

static void ui_apply_mode_color(ui_mode_t mode) {
    if (my_arc == NULL) {
        return;
    }

    lv_obj_set_style_bg_color(my_arc, ui_get_knob_color(mode), LV_PART_KNOB);
}

static void ui_apply_mode_labels(ui_mode_t mode) {
    if (mode_label == NULL || detent_label == NULL) {
        return;
    }

    switch (mode) {
        case UI_MODE_UNBOUNDED_NO_DETENTS:
            lv_label_set_text(mode_label, "Unbounded");
            lv_label_set_text(detent_label, "No detents");
            break;
        case UI_MODE_SOFT_DETENTS:
            lv_label_set_text(mode_label, "Unbounded");
            lv_label_set_text(detent_label, "Soft detents");
            break;
        case UI_MODE_STRONG_DETENTS:
            lv_label_set_text(mode_label, "Unbounded");
            lv_label_set_text(detent_label, "Strong Detents");
            break;
        case UI_MODE_ON_OFF_SWITCH:
            lv_label_set_text(mode_label, "On / Off");
            lv_label_set_text(detent_label, "Snap switch");
            break;
        case UI_MODE_COUNT:
        default:
            break;
    }
}

static void ui_set_primary_text(const char *text) {
    if (value_label != NULL) {
        lv_label_set_text(value_label, text);
    }
}

void ui_init(void) {
    static lv_point_t top_marker_points[] = {
        {UI_ARC_CENTER, 0},
        {UI_ARC_CENTER, UI_KNOB_DIAMETER},
    };

    // 1. Screen Background Object
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    top_marker_line = lv_line_create(screen);
    lv_line_set_points(top_marker_line, top_marker_points, 2);
    lv_obj_set_size(top_marker_line, UI_ARC_SIZE, UI_ARC_SIZE);
    lv_obj_center(top_marker_line);
    lv_obj_set_style_line_color(top_marker_line, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_line_width(top_marker_line, 1, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(top_marker_line, false, LV_PART_MAIN);

    // 2. Create the arc object
    my_arc = lv_arc_create(screen);
    lv_obj_set_size(my_arc, UI_ARC_SIZE, UI_ARC_SIZE);
    lv_obj_center(my_arc);
    lv_arc_set_rotation(my_arc, 270);
    lv_arc_set_bg_angles(my_arc, 0, 360); 
    lv_arc_set_range(my_arc, 0, 360);
    lv_arc_set_value(my_arc, 0);

    // 3. Styling the Arc to look like a Ball in a Track
    lv_obj_set_style_arc_color(my_arc, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_arc_width(my_arc, UI_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_opa(my_arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(my_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(my_arc, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_pad_all(my_arc, UI_KNOB_PAD, LV_PART_KNOB);

    text_container = lv_obj_create(screen);
    lv_obj_remove_style_all(text_container);
    lv_obj_set_size(text_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_layout(text_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(text_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(text_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(text_container, 2, LV_PART_MAIN);
    lv_obj_center(text_container);

    value_label = lv_label_create(text_container);
    lv_label_set_text(value_label, "WELCOME");
    lv_obj_set_style_text_color(value_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    mode_label = lv_label_create(text_container);
    lv_label_set_text(mode_label, "Unbounded");
    lv_obj_set_style_text_color(mode_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    detent_label = lv_label_create(text_container);
    lv_label_set_text(detent_label, "No detents");
    lv_obj_set_style_text_color(detent_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(detent_label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_align(detent_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_add_flag(mode_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(detent_label, LV_OBJ_FLAG_HIDDEN);

    ui_apply_mode_labels(UI_MODE_UNBOUNDED_NO_DETENTS);
    ui_apply_mode_color(UI_MODE_UNBOUNDED_NO_DETENTS);
}

void ui_play_welcome_screen(void) {
    if (my_arc == NULL || value_label == NULL || mode_label == NULL || detent_label == NULL) {
        return;
    }

    ui_set_primary_text("WELCOME");
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_add_flag(mode_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(detent_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_center(text_container);

    for (uint16_t angle = 0; angle <= 360; angle += 4) {
        lv_arc_set_value(my_arc, angle);
        lv_timer_handler();
        sleep_ms(12);
    }

    sleep_ms(250);
    ui_set_center_value(0);
    lv_arc_set_value(my_arc, 0);
    lv_timer_handler();
}

void ui_update_arc(uint16_t value) {
    if (my_arc != NULL) {
        lv_arc_set_value(my_arc, value);
    }
}

void ui_set_mode(ui_mode_t mode) {
    ui_apply_mode_labels(mode);
    ui_apply_mode_color(mode);
    if (text_container != NULL) {
        lv_obj_center(text_container);
    }
}

void ui_set_center_value(uint16_t value) {
    static char label_text[16];
    snprintf(label_text, sizeof(label_text), "%u", value);
    ui_set_center_text(label_text);
}

void ui_set_center_text(const char *text) {
    if (value_label == NULL || mode_label == NULL || detent_label == NULL || text == NULL) {
        return;
    }

    ui_set_primary_text(text);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_clear_flag(mode_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(detent_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_center(text_container);
}
