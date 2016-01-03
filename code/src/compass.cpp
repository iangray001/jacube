#include "Arduino.h"
#include "compass.h"
#include "options.h"

unsigned char current_nose = 1;
unsigned char cached_up;
int cached_bearing;
extern int target_bearing;

//Prototypes for internal helpers
float normaliseBearingRad(float bearing);
int normaliseBearingDeg(int bearing);



/*
 * Get the heading for the magnetometer, based on the currently defined 'nose' face.
 *
 * Returns -1 if the nose face is vertically up or down, else returns the bearing in
 * degrees from magnetic north. Currently tilt compensation and declination corrections
 * are NOT performed.
 *
 * This also caches its result, which is used for subsequent animation
 * calls. If this function is not called periodically, animations
 * will start to get told the side is facing the bearing.
 *
 * Finally, this function calls determineFace() which will cache the upwards face, making
 * this the only function that needs to be periodically called to keep sensor
 * information up to date.
 */
int getHeading(MagnetometerScaled mag_scaled, AccelerometerScaled acc_scaled) {
	float heading = 0;
	int upface, rv;
	boolean invert = false;
	boolean oneeighty = false;

	upface = determineFace(acc_scaled);

	//Check if the nose is face up or down then bearing is meaningless
	if(current_nose == upface || current_nose == oppositeFace(upface)) {
		cached_bearing = -1;
		return -1;
	}

	//The nose is side on, so get the bearing.
	//This is not tilt-compensated
	switch(current_nose) {
	case 0:
		oneeighty = true;
		//no break here
	case 1:
		switch(upface) {
		case 5:
			invert = true;
			//no break here
		case 4:
			heading = atan2(mag_scaled.YAxis, mag_scaled.XAxis);
			break;
		case 3:
			invert = true;
			//no break here
		case 2:
			heading = atan2(mag_scaled.ZAxis, mag_scaled.XAxis);
			break;
		}
		break;

		case 2:
			oneeighty = true;
			//no break here
		case 3:
			switch(upface) {
			case 4:
				invert = true;
				//no break here
			case 5:
				heading = atan2(mag_scaled.XAxis, mag_scaled.YAxis);
				break;
			case 0:
				invert = true;
				//no break here
			case 1:
				heading = atan2(mag_scaled.ZAxis, mag_scaled.YAxis);
				break;
			}
			break;

			case 5:
				oneeighty = true;
				//no break here
			case 4:
				switch(upface) {
				case 2:
					invert = true;
					//no break here
				case 3:
					heading = atan2(mag_scaled.XAxis, mag_scaled.ZAxis);
					break;
				case 1:
					invert = true;
					//no break here
				case 0:
					heading = atan2(mag_scaled.YAxis, mag_scaled.ZAxis);
					break;
				}
				break;
	}

	//Is the compass 180 degrees out?
	if(oneeighty) heading = heading + PI;
	heading = normaliseBearingRad(heading);

	//Is the compass upside down?
	if(invert) heading = 2*PI - heading;
	heading = normaliseBearingRad(heading);

	//Convert to degrees and return
	float headingDegrees = heading * 180/M_PI;
	rv = round(headingDegrees);
	cached_bearing = rv;
	return rv;
}

/*
 * Determine which face of the accelerometer is facing upwards.
 * A simple algorithm is used which seems to work well. The magnitude
 * of the three components is compared to find the largest. Then
 * depending on which component was largest and the sign of that component
 * the "most upward" face can be derived. This also caches its result,
 * which is used for subsequent animation calls.
 *
 * This is called by getHeading, so normally is not required
 * by external code.
 */
char determineFace(AccelerometerScaled scaled) {
	float xa, ya, za;
	unsigned char rv;

	xa = abs(scaled.XAxis);
	ya = abs(scaled.YAxis);
	za = abs(scaled.ZAxis);

	if(xa > ya && xa > za) {
		if(scaled.XAxis > 0) {
			rv = 0;
		} else {
			rv = 1;
		}
	} else if(ya > xa && ya > za) {
		if(scaled.YAxis > 0) {
			rv = 2;
		} else {
			rv = 3;
		}
	} else {
		if(scaled.ZAxis > 0) {
			rv = 4;
		} else {
			rv = 5;
		}
	}

	cached_up = rv;
	return rv;
}

unsigned char getNose() {
	return current_nose;
}

void setNose(unsigned char newnose) {
	current_nose = newnose;
}

unsigned char getTop() {
	return cached_up;
}


/**
 * Is the nose pointing at the target bearing?
 */
boolean pointingCorrectly(int leeway) {
	//DEBUGp("cached_bearing: "); DEBUGp(cached_bearing); DEBUGp("  target_bearing: "); DEBUGln(target_bearing);

	if(cached_bearing == -1) return false;

	int bearingoffset = abs(cached_bearing - target_bearing);
	if(bearingoffset > 180) bearingoffset = abs(bearingoffset - 360);

	//DEBUGp("bearingoffset: "); DEBUGp(bearingoffset); DEBUGp("  leeway: "); DEBUGln(leeway);

	if(bearingoffset > leeway) return false;
	else return true;
}


/*
 * Return the index of the face which is currently closest to the target bearing.
 * If there is no bearing (i.e. the nose is up or down) then returns -1.
 */
char getBearingFace() {
	if(cached_bearing == -1) {
		return -1;
	}

	int bearingoffset = cached_bearing - target_bearing;
	bearingoffset = normaliseBearingDeg(bearingoffset);

	if(bearingoffset >= 315 || bearingoffset < 45) {
		return current_nose;
	} else if(bearingoffset >= 45 && bearingoffset < 135) {
		return anticlockwise(current_nose);
	} else if(bearingoffset >= 135 && bearingoffset < 225) {
		return oppositeFace(current_nose);
	} else if(bearingoffset >= 225 && bearingoffset < 315) {
		return clockwise(current_nose);
	} else {
		return current_nose; //Shouldn't happen
	}
}

/*
 * Fill the buffer sides with the numbers of the 4 faces
 * which are on the side (i.e. not top or bottom)
 */
void getSides(unsigned char* sides) {
	unsigned char p = 0;
	for(unsigned char f = 0; f < 6; f++) {
		if(f == getTop() || f == oppositeFace(getTop())) {
			continue;
		}
		sides[p] = f;
		p++;
	}
}

//The face opposite the provided face
char oppositeFace(char face) {
	switch(face) {
	case 0: return 1;
	case 1: return 0;
	case 2: return 3;
	case 3: return 2;
	case 4: return 5;
	case 5: return 4;
	default: return -1;
	}
}

/*
 * The face clockwise from the provided face, from the perspective of
 * someone looking top-down on the cube. Therefore if the provided
 * face is either top or bottom there is no clockwise and -1 is returned.
 */
unsigned char clockwise(unsigned char side) {
	if(side == cached_up || side == oppositeFace(cached_up)) {
		return -1;
	}

	//This could be recoded as a table.
	switch(cached_up) {
	case 0: {
		switch(side) {
		case 3: return 4;
		case 5: return 3;
		case 2: return 5;
		case 4: return 2;
		}
	} break;
	case 1: {
		switch(side) {
		case 3: return 5;
		case 5: return 2;
		case 2: return 4;
		case 4: return 3;
		}
	} break;
	case 2: {
		switch(side) {
		case 0: return 4;
		case 4: return 1;
		case 1: return 5;
		case 5: return 0;
		}
	} break;
	case 3: {
		switch(side) {
		case 0: return 5;
		case 4: return 0;
		case 1: return 4;
		case 5: return 1;
		}
	} break;
	case 4: {
		switch(side) {
		case 0: return 3;
		case 3: return 1;
		case 1: return 2;
		case 2: return 0;
		}
	} break;
	case 5: {
		switch(side) {
		case 0: return 2;
		case 3: return 0;
		case 1: return 3;
		case 2: return 1;
		}
	} break;
	}

	return -1; //Problem, shouldn't happen
}

/*
 * Which face is anticlockwise from the provided face, from the perspective of
 * someone looking top-down on the cube. Therefore if the provided
 * face is either top or bottom there is no anticlockwise and -1 is returned.
 */
unsigned char anticlockwise(unsigned char side) {
	return clockwise(clockwise(clockwise(side)));
}

/*
 * Normalise a bearing by ensuring it is between 0 and 2*PI radians
 */
float normaliseBearingRad(float bearing) {
	while(bearing < 0) bearing += 2*PI;
	while(bearing >= 2*PI) bearing -= 2*PI;
	return bearing;
}

/*
 * Normalise a bearing by ensuring it is between 0 and 359 degrees
 */
int normaliseBearingDeg(int bearing) {
	while(bearing < 0) bearing += 360;
	while(bearing >= 360) bearing -= 360;
	return bearing;
}


/*
 * Has a magnet been placed next to the sensor to pacify the cube?
 * Assumes the sensors are on and updated!
 */
boolean magneticallyPacified(HMC5883L compass) {
	MagnetometerScaled mag;
	compass.ReadScaledAxis(&mag);
	if(abs(mag.XAxis) > PACIFICATION_THRESHOLD) return true;
	if(abs(mag.YAxis) > PACIFICATION_THRESHOLD) return true;
	if(abs(mag.ZAxis) > PACIFICATION_THRESHOLD) return true;
	return false;
}

