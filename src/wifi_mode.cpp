// Copyright (c) 2019 Lars Baumgaertner

#ifdef USE_WIFI
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include "modem.h"

const uint listening_port = 1666;
WiFiServer tcp_server(listening_port);

WiFiUDP udp;
bool udp_init_done = false;

WiFiClient tcp_client_connection;

void init_wifi()
{
    out_println(String("WiFi Name:     ") + String(WIFI_SSID));
    out_println(String("WiFi PSK:      ") + String(WIFI_PSK));

    WiFi.softAP(WIFI_SSID, WIFI_PSK);
    IPAddress IP = WiFi.softAPIP();
    out_println(String("AP IP address: ") + IP.toString());

    udp.begin(1666);
    udp_init_done = true;

    tcp_server.begin();
}

void check_for_connections()
{
    if (tcp_server.hasClient())
    {
        // If we are already connected to another computer,
        // then reject the new connection. Otherwise accept
        // the connection.
        if (tcp_client_connection.connected())
        {
            Serial.println("Connection rejected");
            tcp_server.available().stop();
        }
        else
        {
            Serial.println("Connection accepted");
            tcp_client_connection = tcp_server.available();
        }
    }
}

void wifi_print(String output)
{
    if (tcp_client_connection.connected())
    {
        tcp_client_connection.print(output);
    }
    if (!udp_init_done)
        return;
    udp.beginPacket("192.168.4.255", listening_port);
    //udp.write((uint8_t *)output.c_str(), output.length());
    udp.print(output);
    udp.endPacket();
}

String tcp_receive_buffer = "";

void wifi_loop_tick()
{
    check_for_connections();

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
        handle_command(cmd);
    }
    if (tcp_client_connection.connected() && tcp_client_connection.available())
    {
        memset(buffer, 0, 512);
        int recv_len = tcp_client_connection.read((uint8_t *)buffer, 512);
        tcp_receive_buffer += buffer;
        if (tcp_receive_buffer.endsWith("\n"))
        {
            tcp_receive_buffer.trim();
            tcp_receive_buffer.toUpperCase();
            Serial.println("TCP Received command: " + tcp_receive_buffer);
            handle_command(tcp_receive_buffer);
            tcp_receive_buffer.remove(0, tcp_receive_buffer.length());
        }
    }
}

#endif