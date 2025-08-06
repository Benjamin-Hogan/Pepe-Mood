# Pepe-Mood

This project demonstrates a minimal LVGL application on the ESP32-2432S028 board using PlatformIO.

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

After flashing, reset the board. The application will initialise LVGL and display a "Hello, LVGL!" label in the centre of the screen.
