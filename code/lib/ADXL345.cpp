/*
 * Initial code based on ADXL345.cpp - Header file for the ADXL345 Triple Axis Accelerometer Arduino Library
 * from Love Electronics (loveelectronics.co.uk)
 *
 * Heavily edited to fix bugs and refactor for code size.
 *
 * Ian Gray, 2013.
 */

#include "Arduino.h"
#include <Wire.h>
#include "ADXL345.h"
#include "i2c_common.h"
#include "../src/options.h"

ADXL345::ADXL345() {
	m_Scale = ScaleFor2G;
	m_Address = DefaultADXL345_Address;
}

ADXL345::ADXL345(uint8_t customAddress) {
	m_Scale = ScaleFor2G;
	m_Address = customAddress;
}

boolean ADXL345::ReadRawAxis(AccelerometerRaw *raw) {
#ifndef ENABLE_SENSORS
	return true;
#endif
	unsigned char buffer[6];
	unsigned char rv = Read(Register_DataX, 6, buffer);

	//If we did not get the data we expected
	if(rv != 6) return false;

	raw->XAxis = (buffer[1] << 8) | buffer[0];
	raw->YAxis = (buffer[3] << 8) | buffer[2];
	raw->ZAxis = (buffer[5] << 8) | buffer[4];
	return true;
}

boolean ADXL345::ReadScaledAxis(AccelerometerScaled *scaled) {
#ifndef ENABLE_SENSORS
	return true;
#endif
	AccelerometerRaw raw;
	if(ReadRawAxis(&raw) == false) {
		return false;
	}
	scaled->XAxis = raw.XAxis * m_Scale;
	scaled->YAxis = raw.YAxis * m_Scale;
	scaled->ZAxis = raw.ZAxis * m_Scale;
	return true;
}

void ADXL345::SetOffset(int x, int y, int z) {
	Write(Register_XOffset, x);
	Write(Register_YOffset, y);
	Write(Register_ZOffset, z);
}


void ADXL345::EnableMeasurements() {
	Write(Register_PowerControl, 0x08);
}

int ADXL345::SetRange(int range, bool fullResolution) {
	//Get current data from this register.
	uint8_t data[1];
	uint8_t rv = Read(Register_DataFormat, 1, data);
	if(rv != 1) {
		return -1;
	}

	//Clearing ----X-XX
	data[0] &= 0xF4;
	m_Scale = ScaleFor2G;

	//Need to set the range bits if requested range is not 2G
	switch(range) {
	case 4:
		data[0] |= 0x01;
		if(!fullResolution) { m_Scale = ScaleFor4G; }
		break;
	case 8:
		data[0] |= 0x02;
		if(!fullResolution) { m_Scale = ScaleFor8G; }
		break;
	case 16:
		data[0] |= 0x03;
		if(!fullResolution) { m_Scale = ScaleFor16G; }
		break;
	}

	// Set the full resolution bit.
	if(fullResolution) data[0] |= 0x08;

	Write(Register_DataFormat, data[0]);
	return 0;
}

uint8_t ADXL345::EnsureConnected() {
	return i2cidentify(m_Address, 0x00, 0xE5);
}

void ADXL345::Write(int address, int data) {
	i2cwrite(m_Address, address, data);
}

int ADXL345::Read(int address, int length, uint8_t *buffer) {
	return i2cread(m_Address, address, length, buffer);
}
