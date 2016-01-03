#ifndef __LEDS_H
#define __LEDS_H

typedef struct _color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Colour;

void initialise_leds();
void next_led_subscan();

void setLED(unsigned char led, unsigned char col);
Colour getLED(unsigned char led);
void setLED(unsigned char led, Colour c);

void setLED(unsigned char led, unsigned char r, unsigned char g, unsigned char b);
void clearLEDs();
void turnOffLEDs();

void start_led_scanner();
void stop_led_scanner();

#endif
