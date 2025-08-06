import time

# MicroPython script to play an MP4 video on loop using LVGL.
# The board ESP32-2432S028 exposes a built-in display handled by lvgl/lvesp32 bindings.
# Ensure that the firmware is built with lvgl and the ffmpeg/mp4 decoder enabled.

import lvgl as lv
import lvesp32

# Initialize LVGL and the display driver shipped with the board
lv.init()
# This helper initializes the ESP32 display and touch drivers
lvesp32.init()

# Set up the video player widget
video = lv.video(lv.scr_act())
video.set_size(320, 240)  # adjust to your display resolution
video.set_src("/sd/video.mp4")  # path to the MP4 file on the SD card
video.set_auto_restart(True)  # loop the clip
video.play()

# Simple main loop keeping LVGL alive
while True:
    lv.task_handler()  # handle LVGL tasks
    time.sleep_ms(5)
