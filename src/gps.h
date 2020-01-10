#ifndef _GPS_H
#define _GPS_H

void init_gps();
void gps_loop_tick();
void print_gps();
void power_gps(bool powerup);
bool gps_enabled();

#endif
