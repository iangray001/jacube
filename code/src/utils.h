#ifndef __UTILS_H_
#define __UTILS_H_

#include "leds.h"

int enable_sensors();
void disable_sensors();
boolean gather_entropy();
void calibration();
void refresh_sensors();

//Configuration
void clear_eeprom();
void write_bearing(int bearing);
int read_bearing();
void write_calibration(int *buf);
void read_calibration(int *buf);

void HSV_to_RGB(float h, float s, float v, byte &r, byte &g, byte &b);
Colour HSV_to_RGB(float h, float s, float v);

void power_sleep_long(int time);

void vibrate_on();
void vibrate_off();

#endif
