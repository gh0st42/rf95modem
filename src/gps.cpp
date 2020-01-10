// Copyright (c) 2018 Lars Baumgaertner

#ifdef USE_GPS
#include <Arduino.h>
#include <TinyGPS++.h>
#include <axp20x.h>

#include "modem.h"
#include "gps.h"

TinyGPSPlus gps;
HardwareSerial GPS1(1);
AXP20X_Class axp;

void init_gps()
{
    Wire.begin(21, 22);
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        Serial.println("AXP192 Begin PASS");
    }
    else
    {
        Serial.println("AXP192 Begin FAIL");
    }
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON); // LORA
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON); // GPS
    // axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON); // OLED
    //axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    //axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);
    //axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
    //axp.setDCDC1Voltage(3300); //esp32 core VDD    3v3
    //axp.setLDO2Voltage(3300);  //LORA VDD set 3v3
    //axp.setLDO3Voltage(3300);  //GPS VDD      3v3

    GPS1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); //17-TX 18-RX
}

void power_gps(bool powerup)
{
    if (powerup)
    {
        axp.setPowerOutPut(AXP192_LDO3, AXP202_ON); // GPS
    }
    else
    {
        axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // GPS
    }
}
bool gps_enabled()
{
    return axp.isLDO3Enable();
}
void print_gps()
{
    if (gps.charsProcessed() < 10)
    {
        out_println("+FAIL: No GPS data received: check wiring");
        return;
    }
    out_print("Latitude  : ");
    out_println(String(gps.location.lat(), 5));
    out_print("Longitude : ");
    out_println(String(gps.location.lng(), 4));
    out_print("Satellites: ");
    out_println(String(gps.satellites.value()));
    out_print("Altitude  : ");
    out_print(String(gps.altitude.feet() / 3.2808));
    out_println("M");
    out_print("Time      : ");
    out_print(String(gps.time.hour()));
    out_print(":");
    out_print(String(gps.time.minute()));
    out_print(":");
    out_println(String(gps.time.second()));
    out_println("+OK");
}

void gps_loop_tick()
{
    if (GPS1.available())
        gps.encode(GPS1.read());
}
#endif