#include "Arduino.h"
#include "options.h"
#include "utils.h"
#include "ADXL345.h"
#include "HMC5883L.h"
#include "compass.h"
#include <EEPROM.h>
#include "leds.h"
#include "Narcoleptic.h"
#include <avr/power.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

extern int target_bearing;
extern ADXL345 accel;
extern int calibration_data[3];
extern HMC5883L compass;

//------------------------------------------------------------------------------------------------------
// Serial and UI functions

/*
 * Read a new bearing from the serial line. Entered in integer degrees as 3 ASCII digits.
 */
void read_new_bearing() {
	char buf[3];
	Serial.print("Enter new bearing: ");

	for(int i = 0; i < 3; i++) {
		while(Serial.available() == 0);
		buf[i] = Serial.read();

		if(!(buf[i] >= '0' && buf[i] <= '9')) {
			Serial.println("Error. Enter as 3 ASCII digits.");
			return;
		}
	}

	int bearing = (buf[0] - '0') * 100 + (buf[1] - '0') * 10 + (buf[2] - '0');
	write_bearing(bearing);
	target_bearing = bearing;
}

//Set a new nose value by reading from the serial line
void setNoseFromSerial() {
	char c = Serial.read();
	c = c - '0';
	if(c >= 0 && c <= 5) {
		setNose(c);
		Serial.print("Nose set to: ");
		Serial.println(getNose());
	}
}

//------------------------------------------------------------------------------------------------------
// Sensor functions

//Power up the sensor board, activate the sensors we need, initialise them, calibrate the accelerometer.
int enable_sensors() {
#ifndef ENABLE_SENSORS
	return 0;
#endif
	AccelerometerScaled acc;
	int cyclecount = 0;

	pinMode(PIN_SENSOR_POWER, OUTPUT);
	digitalWrite(PIN_SENSOR_POWER, HIGH);

	while(1) {
		delay(100);
		cyclecount++;
		TWCR = 0;
		TWCR = TWEN;
		Wire.begin();

		if(!accel.EnsureConnected()) continue;

		if(accel.SetRange(2, true) != 0) { //Accelerometer range, 2Gs
			continue; //Try again
		}

		accel.SetOffset(calibration_data[0], calibration_data[1], calibration_data[2]);
		accel.EnableMeasurements();

		if(accel.ReadScaledAxis(&acc)) {
			float mag = sqrt(acc.XAxis*acc.XAxis + acc.YAxis*acc.YAxis + acc.ZAxis*acc.ZAxis);
			if(mag > 0.5 && mag < 1.5) {
				break;
			}
		}
	}
	compass.SetScale(1.3); //Compass scale +/- 1.3 Ga
	compass.SetMeasurementMode(COMPASS_MEASURE_CONTINUOUS);
	return cyclecount;
}

//Deactivate the sensors
void disable_sensors() {
	//First kill the internal pullups to stop the sensor board leeching power
	//These are re-enabled by Wire.begin()
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega328P__)
	//Deactivate internal pull-ups for twi (atmega8 manual p167)
	cbi(PORTC, 4);
	cbi(PORTC, 5);
#else
	//Deactivate internal pull-ups for twi (atmega128 manual p204)
	cbi(PORTD, 0);
	cbi(PORTD, 1);
#endif
	digitalWrite(PIN_SENSOR_POWER, LOW);
	pinMode(PIN_SENSOR_POWER, INPUT);
}

//Gather entropy for PRNG from the lower bits of the accelerometer
//Activates sensors, checks for their presence.
//Returns false if sensors were not found, true if all OK
boolean gather_entropy() {
	enable_sensors();

	//TODO: Add von Neumann de-bias filter as 0 dominates
	int seed = 0;
	for(int i = 0; i < 32; i++) {
		AccelerometerRaw raw;
		if(accel.ReadRawAxis(&raw) == true) {
			if(raw.XAxis & 1) {
				seed = seed | 1;
			}
			seed = seed << 1;
		} else {
			//Not a big deal in the entropy generator
		}
	}
	randomSeed(seed);
	disable_sensors();
	return true;
}


void refresh_sensors() {
	AccelerometerScaled acc;
	MagnetometerScaled mag;

	if(accel.ReadScaledAxis(&acc) && compass.ReadScaledAxis(&mag)) {
		getHeading(mag, acc);
	}
}


/**
 * Perform calibration of the accelerometer. This assumes that the device is level and with Z upwards.
 * Takes 100 samples, averages them, and writes the appropriate offsets into the offset registers
 * of the accelerometer.
 * This function also saves the determined calibration values into the EEPROM
 */
void calibration() {
	float xa, ya, za;
	enable_sensors();

	//Zero the offset registers first
	accel.SetOffset(0, 0, 0);

	//Take 100 samples
	AccelerometerScaled scaled;
	while(accel.ReadScaledAxis(&scaled) == false); //Loop until a valid sample is read.

	xa = scaled.XAxis;
	ya = scaled.YAxis;
	za = scaled.ZAxis;
	for(int i = 0; i < 100; i++) {
		if(accel.ReadScaledAxis(&scaled)) {
			xa = (xa + scaled.XAxis) / 2.0;
			ya = (ya + scaled.YAxis) / 2.0;
			za = (za + scaled.ZAxis) / 2.0;
		} else {
			//Ignored for this loop
		}
	}
	//Account for expected 1G in the z-axis
	za = za - 1.0;

	//Prepare accelerometer offset values. Values are converted to 1 == 15.6mG
	float xoff, yoff, zoff;
	xoff = xa / 15.6 * 1000;
	yoff = ya / 15.6 * 1000;
	zoff = za / 15.6 * 1000;

	int offset_vals[3];
	offset_vals[0] = -round(xoff);
	offset_vals[1] = -round(yoff);
	offset_vals[2] = -round(zoff);

	//Save calibration data into the EEPROM
	write_calibration(offset_vals);

	//Write the resulting offsets into the chip
	accel.SetOffset(offset_vals[0], offset_vals[1], offset_vals[2]);

	disable_sensors();
}

//-----------------------------------------------------------------------------------
// Configuration

#define EEPROMBYTESUSED 4
#define BEARING_ADDR 0
#define ACC_CALIBRATION_ADDR 1

void clear_eeprom() {
	for(int i = 0; i < EEPROMBYTESUSED; i++) {
		EEPROM.write(i, 0);
	}
}

void write_bearing(int bearing) {
	EEPROM.write(BEARING_ADDR, bearing);
}

int read_bearing() {
	return EEPROM.read(BEARING_ADDR);
}

void write_calibration(int *buf) {
	for(int i = 0; i < 3; i++) {
		EEPROM.write(ACC_CALIBRATION_ADDR + i, buf[i]);
	}
}

void read_calibration(int *buf) {
	for(int i = 0; i < 3; i++) {
		buf[i] = EEPROM.read(ACC_CALIBRATION_ADDR + i);
	}
}



void HSV_to_RGB(float h, float s, float v, byte &r, byte &g, byte &b) {
	int i;
	float f,p,q,t;

	h = constrain(h, 0.0, 360.0);
	s = constrain(s, 0.0, 100.0);
	v = constrain(v, 0.0, 100.0);

	s /= 100;
	v /= 100;

	if (s == 0) {
		// Achromatic (grey)
		r = g = b = round(v*8);
		return;
	}

	h /= 60.0;
	i = floor(h); // sector 0 to 5
	f = h - (float)i; // factorial part of h
	p = v * (1.0 - s);
	q = v * (1.0 - s * f);
	t = v * (1.0 - s * (1 - f));
	switch(i) {
	case 0: r = round(8*v); g = round(8*t);	b = round(8*p);	break;
	case 1: r = round(8*q);	g = round(8*v);	b = round(8*p);	break;
	case 2: r = round(8*p);	g = round(8*v);	b = round(8*t);	break;
	case 3:	r = round(8*p);	g = round(8*q);	b = round(8*v);	break;
	case 4:	r = round(8*t);	g = round(8*p);	b = round(8*v);	break;
	default: // case 5:
		r = round(8*v);	g = round(8*p);	b = round(8*q);
	}
}


Colour HSV_to_RGB(float h, float s, float v) {
	Colour c;
	byte r, g, b;
	HSV_to_RGB(h, s, v, r, g, b);
	c.r = r;
	c.g = g;
	c.b = b;
	return c;
}


//-----------------------------------------------------------------------------------

void vibrate_off() {
	digitalWrite(VIBRATE_ENABLE, LOW);
	pinMode(VIBRATE_ENABLE, INPUT);
}

void vibrate_on() {
	pinMode(VIBRATE_ENABLE, OUTPUT);
	digitalWrite(VIBRATE_ENABLE, HIGH);
}

//Shut down everything and go into deep sleep for a long time.
//Will delay for (time * 8) seconds
void power_sleep_long(int time) {
	//Turn everything off
	Serial.end();
	stop_led_scanner();
	disable_sensors();
	vibrate_off();
	noTone(PIEZO_PIN_1);

	//Tristate and disable pullups on all pins for power saving
	for(int i = 0; i <= 21; i++) {
		pinMode(i, INPUT);
		digitalWrite(i, LOW);
	}

	//And a few further tweaks. Each of these DO save uA. We measured.
	ADCSRA = 0; //ADCs
	power_adc_disable();
	ACSR = B10000000; //Analogue comparator
	//PRR = B11111111; //Internal peripheral power down register
	MCUCR = _BV (BODS) | _BV (BODSE); //Brownout detector (This may have no effect, as we nuked the e-fuses to disable the BDD. But it can't hurt.)
	MCUCR = _BV (BODS);

	Narcoleptic.delay8secs(time);
}
