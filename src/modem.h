#ifndef MODEM_H
#define MODEM_H

#include <Arduino.h>

void modem_setup();
void initRF95();
void handleCommand(String input);
void onpacketreceived(uint8_t *buf, uint8_t len);
void modem_loop_tick();

#endif