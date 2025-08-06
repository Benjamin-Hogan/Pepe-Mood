# Pepe-Mood

This project demonstrates playing an MP4 video on the ESP32-2432S028 board using a C++ application built with PlatformIO and LVGL.

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

## Running the video

1. Convert your video to a resolution supported by the display (e.g. 320x240) and copy the file as `/sd/video.mp4` on an SD card inserted into the board.
2. After flashing, reset the board. The application will initialise LVGL and play the video in a loop.
