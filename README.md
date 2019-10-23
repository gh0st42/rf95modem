# rf95modem MCU firmware
This project provides a modem firmware for arduino boards with a rf95 compatible radio module and a serial interface such as the adafruit feather m0 lora device or the heltec oled lora 32 modules. This branch requires a BLE capable device such as the heltec ESP32 lora boards.

The current default config is for device with 868.1 MHz. The default can be changed in `src/modem.cpp` with the following line: `#define RF95_FREQ 868.1`

## Installation 

The recommended way for building and installing the radio firmware is to have a working installation of platformio (http://platformio.org/) on your system.

*IMPORTANT* Edit platformio.ini to add your target platform and configure the radio pins in the build flags!

Install on your device using `pio run -t upload -e heltec_wifi_lora_32_ble`

Optionally activate display support: `pio run -t upload -e heltec_wifi_lora_32_display_ble`

## BLE notes - CAUTION! BLE functionality - (probably) BUGGY, (maybe) DEFUNCT AND UNDOCUMENTED!

Currently anyone can connect to the BLE service, it is all plaintext. One characteristic is published for sending commands and one is there to make output available via notifications. At the moment any `AT+TX` data is sent directly, according to the specs BLE payload should not exceed 20 bytes. So far, we have also successfully sent data of 100 bytes or more via BLE depending on the platforms involved.

## Modem Usage

List of commands:
```
AT+HELP             Print this usage information.
AT+TX=<hexdata>     Send binary data.
AT+RX=<0|1>         Turn receiving on (1) or off (2).
AT+FREQ=<freq>      Changes the frequency.
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

`AT+TX=414141` sends a packet with `AAA` as content. Maximum packet size may vary depending on radio chip. 

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


### Acknowledging this work

If you use this code in academic publications, please cite the following paper:

```
@INPROCEEDINGS{baumgaertner2018monitoring,
 author={L. {Baumgärtner} and A. {Penning} and P. {Lampe} and B. {Richerzhagen} and R. {Steinmetz} and B. {Freisleben}},
 booktitle={2018 IEEE Global Humanitarian Technology Conference (GHTC)},
 title={Environmental Monitoring Using Low-Cost Hardware and Infrastructureless Wireless Communication},
 year={2018},
 pages={1-8},
 doi={10.1109/GHTC.2018.8601883},
 month={Oct},
}
```