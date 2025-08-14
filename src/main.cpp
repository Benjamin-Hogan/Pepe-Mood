#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include "MjpegClass.h"

// Pin definitions
#define BL_PIN 21  // Backlight pin
#define TFT_BL 21  // Backlight control

// SPI configuration
static const int spi_speed = 80000000;
static const int SD_CS = 5;    // SD Card CS pin
static const int SD_MOSI = 23; // SD Card MOSI (VSPI)
static const int SD_MISO = 19; // SD Card MISO (VSPI)
static const int SD_SCK = 18;  // SD Card SCK (VSPI)

// Video playback settings
#define FPS 30
#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 4)  // Adjusted buffer size
#define VIDEO_PATH "/pepe.mjpeg"

TFT_eSPI tft = TFT_eSPI();
static MjpegClass mjpeg;

// Global variables for performance tracking
static unsigned long total_frames = 0;
static unsigned long total_read_video = 0;
static unsigned long total_decode_video = 0;
static unsigned long total_show_video = 0;
static unsigned long start_ms, curr_ms;

// Buffers for video playback
static uint8_t *mjpeg_buf = nullptr;
static uint16_t *output_buf = nullptr;

// Global variables for MJPEG playback
static int jpegDrawTime = 0;

// Callback function for drawing frames
static int tft_output(JPEGDRAW *pDraw) {
    // Calculate time taken to draw frame
    unsigned long start = millis();
    tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, (uint16_t *)pDraw->pPixels);
    jpegDrawTime += millis() - start;
    return 1; // Return 1 to indicate success
}

// Progress bar function
static void drawProgressBar(int x, int y, int width, int height, uint8_t progress) {
    tft.drawRect(x, y, width, height, TFT_WHITE);
    if (progress > 0) {
        int fillWidth = (width - 2) * progress / 100;
        tft.fillRect(x + 1, y + 1, fillWidth, height - 2, TFT_GREEN);
    }
}

void playVideo(const char *path) {
    Serial.printf("Playing video from: %s\n", path);

    File vFile = SD.open(path);
    if (!vFile || vFile.isDirectory()) {
        Serial.println("ERROR: Failed to open video file");
        return;
    }

    Serial.println("MJPEG video start");
    tft.fillScreen(TFT_BLACK);

    // Reset performance counters
    unsigned long start_ms = millis();
    unsigned long curr_ms;
    unsigned long total_decode_video = 0;
    unsigned long total_play_video = 0;
    total_frames = 0;
    jpegDrawTime = 0;

    // Setup MJPEG decoder
    mjpeg.setup(&vFile, mjpeg_buf, tft_output, false);  // Output little-endian RGB565 pixels

    // Play video
    while (vFile.available()) {
        // Read video
        curr_ms = millis();
        if (!mjpeg.readMjpegBuf()) {
            Serial.println("Read video error");
            break;
        }
        total_play_video += millis() - curr_ms;

        // Play video        
        curr_ms = millis();
        if (!mjpeg.drawJpg()) {
            Serial.println("Draw video error");
            break;
        }
        total_decode_video += millis() - curr_ms;

        total_frames++;

        // Maintain frame rate
        int frame_time = millis() - curr_ms;
        if (frame_time < (1000 / FPS)) {
            delay((1000 / FPS) - frame_time);
        }
    }

    vFile.close();

    // Show statistics
    int total_time = millis() - start_ms;
    float fps = 1000.0 * total_frames / total_time;
    total_decode_video -= jpegDrawTime;

    Serial.printf("Total frames: %d\n", total_frames);
    Serial.printf("Time used: %d ms\n", total_time);
    Serial.printf("Average FPS: %0.1f\n", fps);
    Serial.printf("Read video: %lu ms (%0.1f%%)\n", total_play_video, 100.0 * total_play_video / total_time);
    Serial.printf("Decode video: %lu ms (%0.1f%%)\n", total_decode_video, 100.0 * total_decode_video / total_time);
    Serial.printf("Draw video: %d ms (%0.1f%%)\n", jpegDrawTime, 100.0 * jpegDrawTime / total_time);

    // Show results on screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.printf("Completed %d frames\n", total_frames);
    tft.printf("Avg FPS: %0.1f\n", fps);
    tft.setTextColor(TFT_WHITE);
    tft.printf("Time: %0.1fs\n", total_time / 1000.0);
}

void setup() {
    Serial.begin(115200);

    // Set display backlight pin
    pinMode(BL_PIN, OUTPUT);
    digitalWrite(BL_PIN, HIGH);

    // Initialize display
    Serial.println("Display initialization");
    tft.init();
    tft.setRotation(0); // Portrait
    tft.setSwapBytes(true); // Swap byte order for little-endian pixel data
    tft.invertDisplay(false); // Ensure display color inversion is disabled
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    Serial.printf("Screen size Width=%d, Height=%d\n", tft.width(), tft.height());
    tft.drawString("Initializing SD card...", 10, 10, 2);

    // Initialize SD card using VSPI (same as display)
    const int SD_CS = 5;    // SD Card CS pin
    const int SD_MOSI = 23; // SD Card MOSI (VSPI)
    const int SD_MISO = 19; // SD Card MISO (VSPI)
    const int SD_SCK = 18;  // SD Card SCK (VSPI)

    Serial.println("Initializing SD card on VSPI...");
    Serial.printf("CS:%d MOSI:%d MISO:%d SCK:%d\n", SD_CS, SD_MOSI, SD_MISO, SD_SCK);

    // End any existing connections
    SD.end();
    delay(100);

    // Set up CS pin
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    delay(100);

    // Initialize using default VSPI
    SPIClass sdSPI = SPI;
    sdSPI.begin();

    Serial.println("Attempting SD card initialization...");
    bool sdInitialized = false;

    // Try initialization a few times
    for (int attempt = 0; attempt < 3; attempt++) {
        Serial.printf("SD init attempt %d\n", attempt + 1);

        if (SD.begin(SD_CS, sdSPI)) {
            sdInitialized = true;
            Serial.println("SD.begin() successful");

            // Extra verification steps
            uint8_t cardType = SD.cardType();
            if (cardType == CARD_NONE) {
                Serial.println("No SD card attached");
                sdInitialized = false;
            } else {
                Serial.print("SD Card Type: ");
                if (cardType == CARD_MMC) Serial.println("MMC");
                else if (cardType == CARD_SD) Serial.println("SDSC");
                else if (cardType == CARD_SDHC) Serial.println("SDHC");

                uint64_t cardSize = SD.cardSize() / (1024 * 1024);
                Serial.printf("SD Card Size: %lluMB\n", cardSize);

                // Try to open and read root directory
                File root = SD.open("/");
                if (root) {
                    Serial.println("Root directory opened successfully");
                    if (root.isDirectory()) {
                        File entry = root.openNextFile();
                        bool filesFound = false;
                        while (entry) {
                            filesFound = true;
                            Serial.printf("  Found: %s, size: %d\n", entry.name(), entry.size());
                            entry.close();
                            entry = root.openNextFile();
                        }
                        if (!filesFound) {
                            Serial.println("No files found in root directory");
                        }
                    }
                    root.close();
                } else {
                    Serial.println("Failed to open root directory");
                    sdInitialized = false;
                }
            }
        } else {
            Serial.println("SD.begin() failed");
        }

        if (sdInitialized) break;  // Success - exit attempt loop

        // Clean up before next attempt
        SD.end();
        delay(500);
    }

    if (!sdInitialized) {
        Serial.println("SD card init failed after all attempts");
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("SD card init failed!", 10, 30, 2);
        return;
    }

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("SD card initialized!", 10, 30, 2);
    delay(1000);

    // Allocate video buffers now that the SD card is ready
    Serial.println("Allocating video buffers...");
    mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
    if (!mjpeg_buf) {
        Serial.println("mjpeg_buf allocation failed!");
        while (1) delay(100);
    }

    // Start video playback
    playVideo(VIDEO_PATH);
}

void loop() {
    // Nothing to do here yet
    delay(100); // Small delay to prevent watchdog resets
}
