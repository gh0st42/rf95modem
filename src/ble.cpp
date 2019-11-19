// Copyright (c) 2018 Lars Baumgaertner

#ifdef USE_BLE
#include <Arduino.h>
#include "modem.h"
#include "ble.h"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

String receive_buffer = "";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};
extern struct RF95ModemConfig conf;

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            Serial.println("*********");
            Serial.print("BLE Received Value: ");
            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
                rxValue[i] = ::toupper(rxValue[i]);
            }

            Serial.println();
            Serial.println("*********");

            receive_buffer += rxValue.c_str();

            // Hack to allow BLE devices to activate single big BLE frames without sending a newline
            if (receive_buffer.endsWith("\n") || receive_buffer == "AT+BFB=1" || conf.big_ble_frames == 1)
            {
                receive_buffer.trim();
                Serial.println("BLE Received command: " + receive_buffer);
                handleCommand(receive_buffer);
                receive_buffer.remove(0, receive_buffer.length());
            }
        }

        delay(10);
    }
};

void init_ble()
{
    // Create the BLE Device
    BLEDevice::init("RF95 Modem Service");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);

    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE);

    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
}

void ble_print(String output, byte bcb)
{
    // Sends the whole string at once, according to standard BLE messages should be <20bytes
    // So far communication with iPhone and Raspberry Pi worked also with longer messages
    if (bcb == 1 || output.length() < 20)
    {
        pTxCharacteristic->setValue((uint8_t *)output.c_str(), output.length());
        pTxCharacteristic->notify();
        delay(10);
    }
    else
    {
        //String outputbuf = output;
        while (output.length() != 0)
        {
            unsigned int send_bytes = min((unsigned)20, output.length()); // is this needed?
            String send_data = output.substring(0, send_bytes);           // is substring safe? bounds checking?
            output = output.substring(send_bytes);
            pTxCharacteristic->setValue((uint8_t *)send_data.c_str(), send_data.length());
            pTxCharacteristic->notify();
            delay(10);
        }
    }
}

void ble_loop_tick()
{
    if (deviceConnected)
    {
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
#endif