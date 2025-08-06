# Pepe-Mood

This project demonstrates a minimal LVGL application on the ESP32-2432S028 board using PlatformIO. It initialises the SD card
and stubs out playback of an MP4 file stored at `/videos/demo.mp4` on the card.

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
firmware attempts to read and "play" `/videos/demo.mp4`, reporting progress over the serial port. Actual video decoding is
left to a future implementation.
