#include "Arduino.h"
#include "options.h"
#include "leds.h"
#include "TimerOne.h"

const unsigned char pins[NUMLEDS] = LED_ANODE_PINS;

//The 'frame buffer'
Colour ledbuffer[NUMLEDS];

//Set the pin modes and values that we need
void initialise_leds() {  
	for(unsigned char i = 0; i < NUMLEDS; i++) {
		pinMode(pins[i], OUTPUT);
		digitalWrite(pins[i], LOW);
	}
}

//Attach timer interrupt to begin LED scanning
void start_led_scanner() {
#ifdef USE_LED_SCANNER
	initialise_leds();
	Timer1.initialize(LED_SCANNER_RATE_US);
	Timer1.attachInterrupt(next_led_subscan);
#endif
}

//Stop the timer interrupts
void stop_led_scanner() {
#ifdef USE_LED_SCANNER
	Timer1.stop();
	turnOffLEDs();
#endif
}

//Drive colour pins to 0 to assert, or set to input mode to deassert
//Ensures we do not accidentally drive the LEDs backwards (asserting the colour pins to 1 and the anodes to 0)
void assertColourPin(unsigned char pinnum, boolean assert) {
	if(assert) {
		pinMode(pinnum, OUTPUT);
	} else {
		pinMode(pinnum, INPUT);
	}
}


/*
 * LED scanner with software PWM. Each time this function is called the scanner
 * moves along to the next LED. After scanning the full array of LEDs, it moves
 * back to the start and incremements its scanlevel, which is a variable from 0
 * to LED_SCANMAX. For each LED, its colour channel will be asserted iff the
 * associated colour value in the 'frame buffer' is lower than the current
 * scanlevel.
 *
 * This scanner must be called repeatedly and as close to periodically as
 * possible.
 */
void next_led_subscan() {
	static unsigned char previousled = NUMLEDS - 1;
	static unsigned char scanpos = 0; //Which LED are we currently scanning
	static unsigned char scanlevel = 0; //For software PWM. LED will be lit if its color value is higher than this variable.

	interrupts();

	//Deassert the previous anode
	digitalWrite(pins[previousled], LOW);

	//Set up the colour for the next LED
	assertColourPin(RED_PIN, (scanlevel < ledbuffer[scanpos].r));
	assertColourPin(GREEN_PIN, (scanlevel < ledbuffer[scanpos].g));
	assertColourPin(BLUE_PIN, (scanlevel < ledbuffer[scanpos].b));

	//Activate the current anode
	digitalWrite(pins[scanpos], HIGH);

	//Advance
	previousled = scanpos;
	scanpos++;
	if(scanpos >= NUMLEDS) {
		scanpos = 0;
		scanlevel++;
		if(scanlevel == LED_SCANMAX) scanlevel = 0;
	}
}


void setLED(unsigned char led, unsigned char col) {
	setLED(led, col, col, col);
}

void setLED(unsigned char led, Colour c) {
	if(led < NUMLEDS && led >= 0) {
		ledbuffer[led].r = c.r;
		ledbuffer[led].g = c.g;
		ledbuffer[led].b = c.b;
	}
}

void setLED(unsigned char led, unsigned char r, unsigned char g, unsigned char b) {
	if(led < NUMLEDS && led >= 0) {
		ledbuffer[led].r = r;
		ledbuffer[led].g = g;
		ledbuffer[led].b = b;
	}
}

Colour getLED(unsigned char led) {
	return ledbuffer[led];
}

void clearLEDs() {
	for(unsigned char i = 0; i < NUMLEDS; i++) {
		setLED(i, 0);
	}
}

void turnOffLEDs() {
	for(int i = 0; i < NUMLEDS; i++) digitalWrite(pins[i], LOW);
	pinMode(RED_PIN, INPUT);
	pinMode(GREEN_PIN, INPUT);
	pinMode(BLUE_PIN, INPUT);
}

