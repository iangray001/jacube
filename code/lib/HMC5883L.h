/*
HMC5883L.h - Header file for the HMC5883L Triple Axis Magnetometer Arduino Library.
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

Updated and extended by Ian Gray, 2013.

*/

#ifndef HMC5883L_h
#define HMC5883L_h

#include <inttypes.h>
#include <Wire.h>

#define HMC5883L_Address 0x1E
#define ConfigurationRegisterA 0x00
#define ConfigurationRegisterB 0x01
#define ModeRegister 0x02
#define DataRegisterBegin 0x03
#define IdentityRegister 0x0A
#define IdentityRegisterValue 0x48

#define COMPASS_MEASURE_CONTINUOUS 0x00
#define COMPASS_MEASURE_SINGLESHOT 0x01
#define COMPASS_MEASURE_IDLE 0x03

#define GAUSS_0_88 0
#define GAUSS_1_3  1
#define GAUSS_1_9  2
#define GAUSS_2_5  3
#define GAUSS_4_0  4
#define GAUSS_4_7  5
#define GAUSS_5_6  6
#define GAUSS_8_1  7

#define ErrorCode_1_Num 1

struct MagnetometerScaled {
	float XAxis;
	float YAxis;
	float ZAxis;
};

struct MagnetometerRaw {
	int XAxis;
	int YAxis;
	int ZAxis;
};

class HMC5883L
{
	public:
	  HMC5883L();

	  boolean ReadRawAxis(MagnetometerRaw *raw);
	  boolean ReadScaledAxis(MagnetometerScaled *scaled);
  
	  void SetMeasurementMode(uint8_t mode);
	  boolean SetScale(char gauss);

	  boolean EnsureConnected();
	protected:
	  void Write(int address, int byte);
	  int Read(int address, int length, uint8_t *buffer);

	private:
	  float m_Scale;
};
#endif
