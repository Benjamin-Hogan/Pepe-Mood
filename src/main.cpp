#include <Arduino.h>
#include <lvgl.h>
#include <SD.h>
#include <FS.h>

// The `lvgl_helpers` utilities from the ESP32 specific driver package
// require hardware configuration that isn't provided in this project. To
// keep the example buildable on any system, we only depend on the core LVGL
// library and avoid hardware initialisation here.

// Plays an MP4 file stored on the SD card. This is a stub implementation that
// simply reads the file; actual video decoding and rendering would need a
// dedicated library.

static const char *VIDEO_PATH = "/videos/demo.mp4";
static lv_obj_t *status_label;

void playVideo(const char *path) {
    File video = SD.open(path);
    if (!video) {
        Serial.printf("Failed to open %s\n", path);
        lv_label_set_text(status_label, "Video not found");
        return;
    }

    Serial.printf("Playing %s (%u bytes)\n", path, (unsigned)video.size());

    uint8_t buffer[512];
    while (video.read(buffer, sizeof(buffer)) > 0) {
        // In a real implementation, decode and render frames here.
    }

    video.close();
    Serial.println("Playback finished (stub)");
    lv_label_set_text(status_label, "Playback finished");
}

void setup() {
    Serial.begin(115200);
    lv_init();

    status_label = lv_label_create(lv_scr_act());
    lv_obj_center(status_label);

    if (!SD.begin()) {
        Serial.println("SD card init failed");
        lv_label_set_text(status_label, "SD init failed");
        return;
    }

    lv_label_set_text(status_label, "Playing video...");
    playVideo(VIDEO_PATH);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
