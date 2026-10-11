#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_FORECAST_DATA = 0,
    FLOW_GLOBAL_VARIABLE_IS_METRIC = 1,
    FLOW_GLOBAL_VARIABLE_TIME_DATA = 2,
    FLOW_GLOBAL_VARIABLE_BATTERY_PERCENT = 3,
    FLOW_GLOBAL_VARIABLE_TEMP_DATA = 4,
    FLOW_GLOBAL_VARIABLE_HUMIDITY_DATA = 5,
    FLOW_GLOBAL_VARIABLE_PRESSURE_DATA = 6
};

// Native global variables



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/