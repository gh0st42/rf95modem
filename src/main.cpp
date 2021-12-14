// Copyright (c) 2018, 2019 Lars Baumgaertner

#include <Arduino.h>

#ifdef USE_BLE
#include "ble.h"
#endif

#ifdef USE_WIFI
#include "wifi_mode.h"
#endif

#ifdef USE_GPS
#include "gps.h"
#endif

#include "modem.h"

void setup()
{
#ifndef DESKTOP
#ifdef LED
    pinMode(LED, OUTPUT);
#endif // LED

    Serial.begin(115200);
    //Serial.begin(9600);
    delay(100);
    Serial.setTimeout(2000);
#endif

#ifdef USE_BLE
    init_ble();
#endif

#ifdef USE_WIFI
    init_wifi();
#endif

#ifdef USE_GPS
    init_gps();
#endif

    out_println("rf95modem firmware (v" + String(VERSION) + ")");
    out_println("Copyright (c) 2018, 2019 Lars Baumgaertner");

    modem_setup();
    init_RF95();
    out_println("LoRa radio init OK!");
#ifdef USE_DISPLAY
    init_display();
    print_display();
#endif // USE_DISPLAY
}

void loop()
{
    modem_loop_tick();
#ifdef USE_BLE
    ble_loop_tick();
#endif

#ifdef USE_WIFI
    wifi_loop_tick();
#endif

#ifdef USE_GPS
    gps_loop_tick();
#endif
}
