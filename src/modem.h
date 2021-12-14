#ifndef MODEM_H
#define MODEM_H

#include <Arduino.h>
#include <RH_RF95.h>

#define VERSION "0.7.4"

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.1
//#define RF95_FREQ 434.0

// Singleton configuration struct
struct RF95ModemConfig
{
    RH_RF95::ModemConfigChoice modem_config;
    float frequency;
    byte rx_listen;
    byte big_ble_frames;
};

void out_print(String text);
void out_println(String text);

void modem_setup();
void init_RF95();
void handle_command(String input);
void on_packet_received(uint8_t *buf, uint8_t len);
void modem_loop_tick();
void init_display();
void print_display();
void modem_receive();

#endif
