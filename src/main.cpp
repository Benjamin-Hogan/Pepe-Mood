#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

TFT_eSPI tft = TFT_eSPI();
static const char *VIDEO_PATH = "/videos/pepe-lore.mp4";

void drawProgressBar(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t percent) {
    uint32_t fillWidth = (w * percent) / 100;
    tft.drawRect(x, y, w, h, TFT_WHITE);
    tft.fillRect(x + 1, y + 1, fillWidth - 2, h - 2, TFT_BLUE);
}

void playVideo(const char *path) {
    Serial.printf("Attempting to open video file: %s\n", path);
    
    if (!SD.exists(path)) {
        Serial.println("Video file does not exist!");
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Video file not found!", 10, 120, 2);
        return;
    }
    
    File video = SD.open(path);
    if (!video) {
        Serial.printf("Failed to open %s\n", path);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Error opening video!", 10, 120, 2);
        return;
    }
    
    if (video.size() == 0) {
        Serial.println("Video file is empty!");
        video.close();
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Video file is empty!", 10, 120, 2);
        return;
    }

    uint32_t videoSize = video.size();
    uint32_t bytesRead = 0;
    uint8_t buffer[1024];
    uint32_t lastProgress = 0;
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Playing video...", 10, 100, 2);

    while (video.available()) {
        size_t bytesThisRound = video.read(buffer, sizeof(buffer));
        if (bytesThisRound == 0) break;
        
        bytesRead += bytesThisRound;
        uint8_t progress = (bytesRead * 100) / videoSize;
        
        if (progress != lastProgress) {
            drawProgressBar(10, 150, 300, 20, progress);
            lastProgress = progress;
        }
        
        // In a real implementation, decode and render frames here
        delay(1); // yield to avoid watchdog resets
    }

    video.close();
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Playback finished", 10, 120, 2);
    Serial.println("Playback finished");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    // Initialize backlight first
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    delay(100);  // Give backlight time to stabilize

    // Initialize display
    tft.init();
    tft.setRotation(3);  // Landscape, pins at top
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    delay(500);

    Serial.println("Display initialized");
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
    for(int attempt = 0; attempt < 3; attempt++) {
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
                        while(entry) {
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
    }    if (!sdInitialized) {
        Serial.println("SD card init failed after all attempts");
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("SD card init failed!", 10, 30, 2);
        return;
    }

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("SD card initialized!", 10, 30, 2);
    delay(1000);

    // Start video playback
    playVideo(VIDEO_PATH);
}

void loop() {
    delay(100); // Small delay to prevent watchdog resets
}
