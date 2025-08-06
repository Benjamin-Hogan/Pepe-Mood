#include <Arduino.h>
#include <lvgl.h>
#include <lvgl_helpers.h>

// Simple LVGL application displaying a label

void setup() {
    lv_init();
    lvgl_driver_init();

    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_center(label);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
