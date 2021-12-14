#ifndef _BLE_H
#define _BLE_H

#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

void init_ble();
void ble_loop_tick();
void ble_print(String output, byte bcb);

#endif
