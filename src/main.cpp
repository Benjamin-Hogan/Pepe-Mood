#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SD.h>
#include <FS.h>

// LVGL displays its widgets on the board's ILI9341 screen via the TFT_eSPI
// driver. SD card playback is stubbed out and only reads the file contents.

// Plays an MP4 file stored on the SD card. This is a stub implementation that
// simply reads the file; actual video decoding and rendering would need a
// dedicated library.

static const char *VIDEO_PATH = "/videos/pepe-lore.mp4";
static lv_obj_t *status_label;

TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[320 * 10];

static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

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
        lv_timer_handler();
        lv_tick_inc(1);

        delay(1); // yield to avoid watchdog resets
    }

    video.close();
    Serial.println("Playback finished (stub)");
    lv_label_set_text(status_label, "Playback finished");
}

void setup() {
    Serial.begin(115200);
    lv_init();

    tft.begin();
    tft.setRotation(1); // landscape 320x240

    tft.setSwapBytes(true);
#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif


    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 10);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    lv_disp_drv_register(&disp_drv);

    status_label = lv_label_create(lv_scr_act());
    lv_obj_center(status_label);

    if (!SD.begin(5)) { // SD card CS pin on ESP32-2432S028
        Serial.println("SD card init failed");
        lv_label_set_text(status_label, "SD init failed");
        return;
    }

    lv_label_set_text(status_label, "Playing video...");
    lv_timer_handler(); // draw status label before blocking file read
    playVideo(VIDEO_PATH);
}

void loop() {
    lv_timer_handler();
    lv_tick_inc(5);
    delay(5);
}
