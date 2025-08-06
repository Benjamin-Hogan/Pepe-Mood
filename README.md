# Pepe-Mood

This project demonstrates playing an MP4 video on a loop using MicroPython on the ESP32-2432S028 board. The example relies on LVGL bindings that include the MP4/FFmpeg decoder.

## PlatformIO setup

A basic PlatformIO configuration targeting the board is provided in `platformio.ini`. The configuration does not build firmware; flash a MicroPython image that contains LVGL and MP4 support separately.

To initialise the PlatformIO environment run:

```bash
pio init --board esp32dev
```

## Running the video

1. Convert your video to a resolution supported by the display (e.g. 320x240) and copy the file as `/sd/video.mp4` on an SD card inserted into the board.
2. Upload `src/main.py` to the board using `mpremote` or another MicroPython file transfer tool:
   ```bash
   mpremote cp src/main.py :main.py
   ```
3. Reset the board. The script will automatically start and play the video in a loop.
