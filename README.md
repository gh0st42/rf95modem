# rf95modem
This project provides a modem firmware for arduino boards with a rf95 compatible radio module and a serial interface such as the adafruit feather m0 lora device or the heltec oled lora 32 modules. 

The current default config is for device with 433 MHz.

## Installation 

The recommended way for building and installing the radio firmware is to have a working installation of platformio (http://platformio.org/) on your system.

*IMPORTANT* Edit platformio.ini to add your target platform and configure the radio pins in the build flags!

Install on your device using `pio run -t upload -e heltec_wifi_lora_32`

## Usage

List of commands:
```
AT+HELP             Print this usage information.
AT+TX=<hexdata>     Send binary data.
AT+RX=<0|1>         Turn receiving on (1) or off (2).
AT+INFO             Output status information.
AT+MODE=<NUM>       Set modem config:
                    0 - medium range (default)
                     Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.
                    1 - fast+short range
                     Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.
                    2 - slow+long range
                     Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on.
                    3 - slow+long range
                     Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on.
```

### Sending data

`AT+TX=414141` sends a packet with AAA as content. Maximum packet size may vary depending on radio chip. 

### Receiving data

`AT+RX=1` activate receive listener, default is on.

Incoming data is automatically written to serial port: `+RX 3,414141,-15,8` - A frame with "AAA" as payload was received with RSSI of -15 and SNR of 8.

### Getting status information

`AT+INFO` displays various status information such as current configuration, frequency and version of radio modem firmware. Output looks like the following:
```
status info:

firmware:      0.1
modem config:  medium range
max pkt size:  251
frequency:     433.00
rx listener:   1

rx bad:        0
rx good:       0
tx good:       3
```
