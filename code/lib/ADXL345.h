/*
ADXL345.h - Header file for the ADXL345 Triple Axis Accelerometer Arduino Library.
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

 Datasheet for ADXL345:
 http://www.analog.com/static/imported-files/data_sheets/ADXL345.pdf

Updated and extended by Ian Gray, 2013.

*/

#ifndef ADXL345_h
#define ADXL345_h

#include <Wire.h>

#define DefaultADXL345_Address 0x1D

#define Register_PowerControl 0x2D
#define Register_DataFormat 0x31
#define Register_DataX 0x32
#define Register_DataY 0x34
#define Register_DataZ 0x36

#define Register_XOffset 0x1E
#define Register_YOffset 0x1F
#define Register_ZOffset 0x20

#define ScaleFor2G 0.0039
#define ScaleFor4G 0.0078
#define ScaleFor8G 0.0156
#define ScaleFor16G 0.0312

struct AccelerometerScaled {
	float XAxis;
	float YAxis;
	float ZAxis;
};

struct AccelerometerRaw {
	int XAxis;
	int YAxis;
	int ZAxis;
};

class ADXL345 {
	public:
	  ADXL345();
	  ADXL345(unsigned char customAddress);
	  boolean ReadRawAxis(AccelerometerRaw *raw);
	  boolean ReadScaledAxis(AccelerometerScaled *scaled);
	  int SetRange(int range, bool fullResolution);
	  void EnableMeasurements();
	  void SetOffset(int x, int y, int z);
	  unsigned char EnsureConnected();

	protected:
	  void Write(int address, int byte);
	  int Read(int address, int length, unsigned char *buffer);

	private:
	  int m_Address;
	  float m_Scale;
};
#endif
