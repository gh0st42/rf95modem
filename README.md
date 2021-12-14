# rf95modem MCU firmware
This project provides a modem firmware for microcontroller boards with a RF95 compatible radio module and a serial interface such as the adafruit feather m0 lora device or the heltec oled lora 32 modules. On various ESP32 based boards optional features such as OLED status display, GPS, BLE or WiFi support can be enabled.

The current default config is for device with 868.1 MHz. 
This can be changed in `src/modem.h` with the following line: `#define RF95_FREQ 868.1`

## Installation 

The recommended way for building and installing the radio firmware is to have a working installation of platformio (http://platformio.org/) on your system.

*IMPORTANT* Edit `platformio.ini` to add your target platform and configure the radio pins in the build flags!

Install on your device using `pio run -t upload -e heltec_wifi_lora_32_ble`

Optionally activate display support: `pio run -t upload -e heltec_wifi_lora_32_display_ble`

## BLE notes - CAUTION! BLE functionality - (probably) BUGGY, (maybe) DEFUNCT AND UNDOCUMENTED!

Currently anyone can connect to the BLE service, it is all plaintext. One characteristic is published for sending commands and one is there to make output available via notifications. 

All commands sent via BLE must be terminated with an `\n`. Default mode of operation is splitting everything into 20 byte chunks, which - according to the BLE specs - is the maximum packet size. On iPhone 8 & 11 we were also able to send and receive much larger BLE packets (>100bytes). Therefore, one can activate *Big Funky BLE-Frames* mode via `AT+BFB=1`. The command is recognized even without trailing `\n` and also makes `\n` optional. This is especially useful as some BLE debugging software such as *LightBlue Explorer* does not send carriage returns or line feeds at the end of a write operation.

## WiFi notes - Experimental LoRa modem via TCP and UDP!

If one of the WiFi profiles is installed on a compatible ESP MCU the device can act as an access point. 
The credentials are configured in `platformio.ini` and are by default set to: `WIFI_SSID=\"rf95modem\"` and `WIFI_PSK=\"rf95modemwifi\"`
This access point accepts up to 4 clients according to espressif sdk and by default has the IP `192.168.4.1`. 

There are two ways to communicate with the modem:

### UDP Broadcasts

The rf95modem responds to UDP broadcast packets to port `1666`.
To receive output a simple udp listener is provided (`extras/udp_receiver.py`). 
For sending commands to the modem netcat is sufficient, e.g. `echo "at+tx=414141" | ncat -u 192.168.4.255 1666`

### Single TCP Connection

Just connect to `192.168.4.1` on port `1666` using TCP and use it like a serial connection, e.g. `ncat 192.168.4.1 1666`. 

**Only one connection at a time is supported!**

## GPS Support 

Some devices such as TTGOs t-beam also include a GPS chip. 
This can also be queried through the modem firmware through the `AT+GPS` command. 
Getting an initial lock for your position can take several minutes depending on your atenna, position and the GPS chip on the device. 
As this also increases energy consumption significantly (~50mA) one can temporarily disable it via `AT+GPS=0`.

## Modem Usage

The default serial speed is set to 115200 (`src/main.cpp` *line 17*).

List of commands:
```
AT+HELP             Print this usage information.
AT+TX=<hexdata>     Send binary data.
AT+RX=<0|1>         Turn receiving on (1) or off (2).
AT+BFB=<0|1>        Turn send Big Fine BLE-Frames on (1) or off (0).
AT+GPS              Print GPS information.
AT+GPS=<0|1>        Turn GPS power on (1) or off (0).
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

### Configuring the Modem

To get the current configuration one can use `AT+INFO`

```
> AT+INFO
+STATUS:

firmware:      0.7.3
features:      MCU BLE WIFI GPS
modem config:  0 | medium range
max pkt size:  251
frequency:     868.10
rx listener:   1
BFB:           0
GPS:           1

rx bad:        0
rx good:       0
tx good:       0
+OK
```

From this output you can see which features where compiled into the firmware and what its version is. 
Also the current modem configuration and the frequency selected are displayed.

To change frequencies on can use the `AT+FREQ` command.

```
> AT+FREQ=868.20
+FREQ: 868.20
```

Beware: Any float number can be added here, the value is directly passed to the LoRa transceiver!

Changing the preconfigured modem mode is just as easy:
```
> AT+MODE=2
+OK
```

### Sending data

`AT+TX=414141` sends a packet with `AAA` as content. Maximum packet size may vary depending on radio chip. 

### Receiving data

`AT+RX=1` activate receive listener, default is on.

Incoming data is automatically written to serial port: `+RX 3,414141,-15,8` - A frame with "AAA" as payload was received with RSSI of -15 and SNR of 8.

### Getting the Location

If GPS is activated (`AT+GPS=1`) and the firmware is running on a GPS-capable device such as the TTGO t-beam, one can easily query the current location and time.
Without a propoer GPS lock all values returned are set to zero.

```
> at+gps=1
+OK
> at+gps
Latitude  :  0.00000
Longitude :  0.00000
Altitude  : 0.00M
Satellites: 0
Time      : 00:00:00
Date      : 00.00.2000
Timestamp : 943920000
+OK
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
