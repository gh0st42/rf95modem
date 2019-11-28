// Copyright (c) 2019 Lars Baumgaertner

#ifdef USE_WIFI
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "modem.h"

WiFiUDP udp;
bool udp_init_done = false;

void init_wifi()
{
    out_println(String("WiFi Name:     ") + String(WIFI_SSID));
    out_println(String("WiFi PSK:      ") + String(WIFI_PSK));

    WiFi.softAP(WIFI_SSID, WIFI_PSK);
    IPAddress IP = WiFi.softAPIP();
    out_println(String("AP IP address: ") + IP.toString());

    udp.begin(1666);
    udp_init_done = true;
}

void wifi_print(String output)
{
    if (!udp_init_done)
        return;
    udp.beginPacket("192.168.4.255", 1666);
    //udp.write((uint8_t *)output.c_str(), output.length());
    udp.print(output);
    udp.endPacket();
}

void wifi_loop_tick()
{
    char buffer[512];
    memset(buffer, 0, 512);
    udp.parsePacket();
    int recv_len = udp.read(buffer, 512);
    if (recv_len > 0)
    {
        String cmd = String(buffer);
        cmd.trim();
        cmd.toUpperCase();
        Serial.println("WiFi Received command: " + cmd);
        handleCommand(cmd);
    }
}

#endif