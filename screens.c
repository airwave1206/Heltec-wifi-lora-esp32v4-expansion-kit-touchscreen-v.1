#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;

static void event_handler_cb_main_met_imp_switch(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            assignBooleanProperty(flowState, 10, 3, value, "Failed to assign Checked state");
        }
    }
}

void create_screen_main() {
    void *flowState = getFlowState(0, 0);
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_set_style_bg_image_src(obj, &img_beach_background, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // time_data
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.time_data = obj;
            lv_obj_set_pos(obj, 130, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // temp_data
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.temp_data = obj;
            lv_obj_set_pos(obj, 166, 60);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff2b04f1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // humidity_data
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.humidity_data = obj;
            lv_obj_set_pos(obj, 166, 88);
            lv_obj_set_size(obj, 144, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff21d41d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // pressure_data
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.pressure_data = obj;
            lv_obj_set_pos(obj, 166, 116);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffeb0526), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // forecast_data
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.forecast_data = obj;
            lv_obj_set_pos(obj, 166, 145);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffbf04f1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // temp_lable
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.temp_lable = obj;
            lv_obj_set_pos(obj, 85, 60);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Temp");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff5e5f62), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // humidity_lable
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.humidity_lable = obj;
            lv_obj_set_pos(obj, 52, 88);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Humidity");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff5e5f62), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // pressure_lable
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.pressure_lable = obj;
            lv_obj_set_pos(obj, 58, 116);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Pressure");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff5e5f62), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // forecast_lable
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.forecast_lable = obj;
            lv_obj_set_pos(obj, 60, 145);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Forecast");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff5e5f62), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // met_imp_switch
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.met_imp_switch = obj;
            lv_obj_set_pos(obj, 132, 211);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, event_handler_cb_main_met_imp_switch, LV_EVENT_ALL, flowState);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a4eb9), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff4eeea7), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_main_stop(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a4eb9), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff4eeea7), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_main_stop(obj, 10, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff23cdc6), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a4eb9), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff4eeea7), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff1a4eb9), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff4eeea7), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_main_stop(obj, 10, LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff38e7d8), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff4eeea7), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_ofs_y(obj, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0xff494545), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff24c410), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // imp_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.imp_label = obj;
            lv_obj_set_pos(obj, 195, 215);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Imperial");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff0e3ed8), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // met_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.met_label = obj;
            lv_obj_set_pos(obj, 72, 215);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Metric");
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffe31444), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // BATTERY_PERCENT
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.battery_percent = obj;
            lv_obj_set_pos(obj, 20, 30);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff1045b), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Battery
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.battery = obj;
            lv_obj_set_pos(obj, 10, 10);
            lv_obj_set_size(obj, 54, 16);
            lv_label_set_text(obj, "Battery");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 7, 9);
            lv_obj_set_size(obj, 60, 40);
            lv_label_set_text(obj, "");
            lv_obj_set_style_border_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_clip_corner(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    void *flowState = getFlowState(0, 0);
    {
        const char *new_val = evalTextProperty(flowState, 0, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.time_data);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.time_data;
            lv_label_set_text(objects.time_data, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 2, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.temp_data);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.temp_data;
            lv_label_set_text(objects.temp_data, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 3, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.humidity_data);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.humidity_data;
            lv_label_set_text(objects.humidity_data, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 4, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.pressure_data);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.pressure_data;
            lv_label_set_text(objects.pressure_data, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 5, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.forecast_data);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.forecast_data;
            lv_label_set_text(objects.forecast_data, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 10, 3, "Failed to evaluate Checked state");
        bool cur_val = lv_obj_has_state(objects.met_imp_switch, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.met_imp_switch;
            if (new_val) lv_obj_add_state(objects.met_imp_switch, LV_STATE_CHECKED);
            else lv_obj_clear_state(objects.met_imp_switch, LV_STATE_CHECKED);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 13, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.battery_percent);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.battery_percent;
            lv_label_set_text(objects.battery_percent, new_val);
            tick_value_change_obj = NULL;
        }
    }
}


static const char *screen_names[] = { "Main" };
static const char *object_names[] = { "main", "met_imp_switch", "time_data", "temp_data", "humidity_data", "pressure_data", "forecast_data", "temp_lable", "humidity_lable", "pressure_lable", "forecast_lable", "imp_label", "met_label", "battery_percent", "battery", "obj0" };


typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
