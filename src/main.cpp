#include <Arduino.h>
#include <lvgl.h>
#include <lvgl_helpers.h>
#include <lv_video.h>

// C++ application playing an MP4 video from the SD card using LVGL

void setup() {
    lv_init();
    lvgl_driver_init();

    lv_obj_t * video = lv_video_create(lv_scr_act());
    lv_obj_set_size(video, 320, 240);
    lv_video_set_src(video, "/sd/video.mp4");
    lv_video_set_auto_restart(video, true);
    lv_video_play(video);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
