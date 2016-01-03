#include "i2c_common.h"
#include "Arduino.h"
#include <Wire.h>

int i2cread(int device, int address, int length, unsigned char *buffer) {
	int rv = 0;
	Wire.beginTransmission(device);
	Wire.write(address);
	Wire.endTransmission();

	Wire.beginTransmission(device);
	Wire.requestFrom(device, length);

	if(Wire.available() == length) {
		rv = length;
		for(unsigned char i = 0; i < length; i++) {
			buffer[i] = Wire.read();
		}
	}
	Wire.endTransmission();
	return rv;
}

void i2cwrite(int device, int address, int data) {
	Wire.beginTransmission(device);
	Wire.write(address);
	Wire.write(data);
	Wire.endTransmission();
}

boolean i2cidentify(int device, unsigned char idregister, unsigned char regvalue) {
	unsigned char data[1];
	int rv = i2cread(device, idregister, 1, data);
	if(rv > 0 && data[0] == regvalue) return true;
	else return false;
}
