#ifndef __COMPASS_H_
#define __COMPASS_H_

#include "ADXL345.h"
#include "HMC5883L.h"

int getHeading(MagnetometerScaled mag_scaled, AccelerometerScaled acc_scaled);
char determineFace(AccelerometerScaled scaled);
char oppositeFace(char face);

//Nose manipulation ;)
unsigned char getNose();
void setNose(unsigned char newnose);

//Game helpers
boolean pointingCorrectly(int leeway);
boolean magneticallyPacified(HMC5883L compass);

//Animation helper functions
void getSides(unsigned char* sides);
unsigned char getTop();
char getBearingFace();
unsigned char clockwise(unsigned char side);
unsigned char anticlockwise(unsigned char side);

extern int cached_bearing;

#endif
