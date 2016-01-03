/*
HMC5883L.cpp - Class file for the HMC5883L Triple Axis Magnetometer Arduino Library.
Copyright (C) 2011 Love Electronics (loveelectronics.co.uk)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

 WARNING: THE HMC5883L IS NOT IDENTICAL TO THE HMC5883!
 Datasheet for HMC5883L:
 http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/HMC5883L_3-Axis_Digital_Compass_IC.pdf
 http://c48754.r54.cf3.rackcdn.com/HMC5883L.pdf


Updated, extended (and fixed) by Ian Gray, 2013.

 */

#include "Arduino.h"
#include <Wire.h>
#include "HMC5883L.h"
#include "i2c_common.h"
#include "../src/options.h"

HMC5883L::HMC5883L() {
	m_Scale = 0.73;
}

boolean HMC5883L::ReadRawAxis(MagnetometerRaw *raw) {
#ifndef ENABLE_SENSORS
	return true;
#endif
	uint8_t buffer[6];
	uint8_t rv = Read(DataRegisterBegin, 6, buffer);

	//If we did not get the data we expected
	if(rv != 6) return false;

	raw->XAxis = (buffer[0] << 8) | buffer[1];
	raw->ZAxis = (buffer[2] << 8) | buffer[3];
	raw->YAxis = (buffer[4] << 8) | buffer[5];
	return true;
}

boolean HMC5883L::ReadScaledAxis(MagnetometerScaled *scaled) {
#ifndef ENABLE_SENSORS
	return true;
#endif
	MagnetometerRaw raw;
	if(ReadRawAxis(&raw) == false) {
		return false;
	}
	scaled->XAxis = raw.XAxis * m_Scale;
	scaled->ZAxis = raw.ZAxis * m_Scale;
	scaled->YAxis = raw.YAxis * m_Scale;
	return true;
}

boolean HMC5883L::SetScale(char gauss) {
	uint8_t regValue = gauss;

	switch(gauss) {
	case GAUSS_0_88: m_Scale = 0.73; break;
	case GAUSS_1_3: m_Scale = 0.92; break;
	case GAUSS_1_9: m_Scale = 1.22; break;
	case GAUSS_2_5: m_Scale = 1.52; break;
	case GAUSS_4_0: m_Scale = 2.27; break;
	case GAUSS_4_7: m_Scale = 2.56; break;
	case GAUSS_5_6: m_Scale = 3.03; break;
	case GAUSS_8_1: m_Scale = 4.35; break;
	default: return false;
	}

	// Setting is in the top 3 bits of the register.
	regValue = regValue << 5;
	Write(ConfigurationRegisterB, regValue);
	return true;
}

void HMC5883L::SetMeasurementMode(uint8_t mode) {
	Write(ModeRegister, mode);
}

boolean HMC5883L::EnsureConnected() {
	return i2cidentify(HMC5883L_Address, IdentityRegister, IdentityRegisterValue);
}

void HMC5883L::Write(int address, int data) {
	i2cwrite(HMC5883L_Address, address, data);
}

int HMC5883L::Read(int address, int length, uint8_t *buffer) {
	return i2cread(HMC5883L_Address, address, length, buffer);
}

