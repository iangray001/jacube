#ifndef __I2C_COMMON_H_
#define __I2C_COMMON_H_

#include <Arduino.h>

int i2cread(int device, int address, int length, unsigned char *buffer);
void i2cwrite(int device, int address, int data);
boolean i2cidentify(int device, unsigned char idregister, unsigned char regvalue);

#endif
