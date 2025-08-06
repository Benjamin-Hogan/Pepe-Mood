# Pepe-Mood


This project demonstrates a minimal LVGL application on the ESP32-2432S028 board using PlatformIO. It registers an LVGL display driver backed by [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) for the board's ILI9341 panel, initialises the SD card, and stubs out playback of an MP4 file stored at `/videos/pepe-lore.mp4` on the card.



## Building

1. Install [PlatformIO](https://platformio.org/).
2. Build the firmware:
   ```bash
   pio run
   ```
3. Upload the firmware to the board:
   ```bash
   pio run --target upload
   ```

## Running

Copy an MP4 file to the `videos` directory on an SD card and insert it into the board. After flashing and resetting, the
firmware attempts to read and "play" `/videos/pepe-lore.mp4`, reporting progress over the serial port. Actual video decoding is
left to a future implementation.

`TFT_eSPI` must be configured for the ESP32-2432S028 (Cheap Yellow Display). Define `ILI9341_DRIVER` and set the following pins in your `User_Setup.h`:

```
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define TFT_BL   21
#define SPI_FREQUENCY 40000000
```
