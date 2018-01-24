# rf95modem
a modem firmware for arduino boards with a rf95 compatible radio module and a serial interface

The current default config is for adafruit feather m0 lora device with 433 MHz.

*IMPORTANT* Edit platformio.ini to add your target platform and configure the radio pins in main.cpp!

Install on your device using `pio run -t upload`