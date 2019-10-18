/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    Modified by Lars Baumgaertner to act as bluetooth LoRa modem

*/
#include <Arduino.h>

#ifdef BLE
#include "ble.h"
#endif

#include "modem.h"

void setup()
{
    //Serial.begin(115200);
    Serial.begin(9600);
#ifdef BLE
    init_ble();
#endif
    modem_setup();
    initRF95();
}

void loop()
{
    modem_loop_tick();
#ifdef BLE
    ble_loop_tick();
#endif
}