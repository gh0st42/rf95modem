#ifndef MODEM_H
#define MODEM_H

#include <Arduino.h>

#define VERSION "0.5"

void out_print(String text);
void out_println(String text);

void modem_setup();
void initRF95();
void handleCommand(String input);
void onpacketreceived(uint8_t *buf, uint8_t len);
void modem_loop_tick();
void initDisplay();
void printDisplay();

#endif