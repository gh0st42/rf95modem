#ifndef _WIFI_H
#define _WIFI_H

#include <Arduino.h>

void init_wifi();
void wifi_loop_tick();
void wifi_print(String output);

#endif
