#include <Arduino.h>
#include <lvgl.h>
// The `lvgl_helpers` utilities from the ESP32 specific driver package
// require hardware configuration that isn't provided in this project. To
// keep the example buildable on any system, we only depend on the core LVGL
// library and avoid hardware initialisation here.

// Simple LVGL application displaying a label

void setup() {
    lv_init();

    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_center(label);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
