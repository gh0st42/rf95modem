// Copyright (c) 2018 Lars Baumgaertner

#include <Arduino.h>

#ifdef USE_BLE
#include "ble.h"
#endif

#include "modem.h"

void setup()
{
#ifdef LED
    pinMode(LED, OUTPUT);
#endif // LED

    //Serial.begin(115200);
    Serial.begin(9600);
    delay(100);
    Serial.setTimeout(2000);

#ifdef USE_BLE
    init_ble();
#endif
    out_println("rf95modem firmware (v" + String(VERSION) + ")");
    out_println("Copyright (c) 2018, 2019 Lars Baumgaertner");

    modem_setup();
    initRF95();
    out_println("LoRa radio init OK!");
#ifdef USE_DISPLAY
    initDisplay();
    printDisplay();
#endif // USE_DISPLAY
}

void loop()
{
    modem_loop_tick();
#ifdef USE_BLE
    ble_loop_tick();
#endif
}