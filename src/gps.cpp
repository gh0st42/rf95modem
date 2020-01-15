// Copyright (c) 2018 Lars Baumgaertner

#ifdef USE_GPS
#include <Arduino.h>
#include <TinyGPS++.h>
#include <axp20x.h>
#include <time.h>

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
#define PAD(X, STR) \
    if (X < 10)     \
    out_print(STR)

void print_gps()
{
    if (gps.charsProcessed() < 10)
    {
        out_println("+FAIL: No GPS data received: check wiring");
        return;
    }
    tm t;
    t.tm_sec = gps.time.second();
    t.tm_min = gps.time.minute();
    t.tm_hour = gps.time.hour();
    t.tm_mday = gps.date.day();
    t.tm_mon = gps.date.month() - 1;    // 2-1, not 2!
    t.tm_year = gps.date.year() - 1900; // 1990-1900, not 1990!
    t.tm_isdst = -1;
    out_print("Latitude  : ");
    PAD(gps.location.lat(), " ");
    out_println(String(gps.location.lat(), 5));
    out_print("Longitude : ");
    PAD(gps.location.lng(), " ");
    out_println(String(gps.location.lng(), 5));
    out_print("Altitude  : ");
    out_print(String(gps.altitude.meters()));
    out_println("M");
    out_print("Satellites: ");
    out_println(String(gps.satellites.value()));
    out_print("Time      : ");
    PAD(gps.time.hour(), "0");
    out_print(String(gps.time.hour()));
    out_print(":");
    PAD(gps.time.minute(), "0");
    out_print(String(gps.time.minute()));
    out_print(":");
    PAD(gps.time.second(), "0");
    out_println(String(gps.time.second()));
    out_print("Date      : ");
    PAD(gps.date.day(), "0");
    out_print(String(gps.date.day()));
    out_print(".");
    PAD(gps.date.month(), "0");
    out_print(String(gps.date.month()));
    out_print(".");
    out_println(String(gps.date.year()));
    out_print("Timestamp : ");
    out_println(String(mktime(&t)));
    out_println("+OK");
}

void gps_loop_tick()
{
    if (GPS1.available())
        gps.encode(GPS1.read());
}
#endif