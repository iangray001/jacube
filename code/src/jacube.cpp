#include "Arduino.h"
#include "options.h"
#include "ADXL345.h"
#include "HMC5883L.h"
#include "animations.h"
#include "compass.h"
#include "leds.h"
#include "utils.h"
#include "notes.h"
#include "Narcoleptic.h"

ADXL345 accel;
HMC5883L compass;
int target_bearing = 0;
int calibration_data[3];

//Prototypes
void mainMode();
void playHappyAnim(int select = -1);

//---------------------------------------------------------------------------------

const prog_char misland[] PROGMEM = "m:d=4,o=5,b=220:8e6,8p,8e6,8g6,8f#6,8e6,d6,2e6,8p,8d6,8p,8d6,8c6,8b,8d6,8c6,8p,8c6,8p,b";
const prog_char close5[] PROGMEM = "c:d=16,o=5,b=140:d,p,e,p,c,p,c4,p,g4";
const prog_char triad[] PROGMEM = "a:d=16,o=5,b=200:c,32p,e,32p,g";

//---------------------------------------------------------------------------------

void setup() {
	Serial.begin(BAUD_RATE);

	//Fetch configuration from EEPROM
	//read_calibration(calibration_data);

	//Hardcoded calibration data instead now that we are happy it is stable
	calibration_data[0] = 12;
	calibration_data[1] = 12;
	calibration_data[2] = 24;

	target_bearing = read_bearing();


	if(!DEBUG) {
		char buf[2];
		Serial.print("Hello! The current target bearing is ");
		Serial.println(target_bearing);
		Serial.println("To change press any key in 1 second.");
		delay(1000);
		if(Serial.available() > 0) {
			while(Serial.available() > 0) Serial.read();

			Serial.println("OK. Enter the new bearing as three ASCII digits now: ");
			for(int i = 0; i < 3; i++) {
				while(Serial.available() == 0);
				buf[i] = Serial.read();
				Serial.print(buf[i]);
			}

			int newbearing = (buf[0]-'0')*100 + (buf[1]-'0')*10 + (buf[2]-'0');
			write_bearing(newbearing);
			target_bearing = newbearing;
			Serial.print("New target bearing is ");
			Serial.println(target_bearing);
		} else {
			Serial.println("Continuing to run...");
		}
	}

	if(DEBUG) {
		Serial.print("Calibration data: X ");
		Serial.print(calibration_data[0]);
		Serial.print(", Y ");
		Serial.print(calibration_data[1]);
		Serial.print(", Z ");
		Serial.println(calibration_data[2]);
		Serial.print("Target bearing: ");
		Serial.println(target_bearing);
	}

	//Create the driver objects
	accel = ADXL345(ADXL345_ADDRESS);
	compass = HMC5883L(); //The magnetometer is on its default address 0x1E

	//Gather entropy for the random number generator
	gather_entropy();
}

//---------------------------------------------------------------------------------

void loop() {
	if(DEBUG) debugMode();
	else mainMode();
}

//---------------------------------------------------------------------------------

void mainMode() {
	int unhappiness = 0;
	int animsToNoseChange = BEHAVIOUR_ANIMS_BETWEEN_NOSE_CHANGES;
	int timeBetweenAnims = BEHAVIOUR_TIME_BETWEEN_ANIMS_8SECS;

	while(1) {
		//Check if we are currently correctly oriented
		enable_sensors();
		refresh_sensors();

		if(!magneticallyPacified(compass)) {

			Serial.begin(BAUD_RATE);

			if(pointingCorrectly(BEHAVIOUR_BEARING_LEEWAY)) {
				//Happy
				DEBUGp("Happy - timeBetweenAnims: "); DEBUGp(timeBetweenAnims); DEBUGp("  animsToNoseChange: "); DEBUGln(animsToNoseChange);
				unhappiness = 0;

				if(timeBetweenAnims <= 0) {
					//Play a "happy" animation
					playHappyAnim();
					timeBetweenAnims = BEHAVIOUR_TIME_BETWEEN_ANIMS_8SECS;
					animsToNoseChange--;
				} else {
					timeBetweenAnims--;
				}

				if(animsToNoseChange <= 0) {
					//Change the nose (i.e. trigger a freak out)
					unsigned char newnose = random(0, 6);
					while(newnose == getNose()) newnose = random(0, 6);
					setNose(newnose);

					DEBUGp("Nose changed to "); DEBUGln(newnose);

					//How many animations before it will change the nose again?
					animsToNoseChange = BEHAVIOUR_ANIMS_BETWEEN_NOSE_CHANGES;
				}

				power_sleep_long(1); //Sleep for a bit

			} else {
				//Unhappy
				DEBUGln("Unhappy");
				//In this mode we will freak out, progressively more and more, until it is correctly oriented again
				//However if we get apoplectic its possible the owner is out, so calm down for a bit.
				Unhappy unhappyAnim;
				play_animation(&unhappyAnim, -1);

				HappyAnim hap;
				play_animation(&hap, -1);
				timeBetweenAnims = BEHAVIOUR_TIME_BETWEEN_ANIMS_8SECS;
				animsToNoseChange = BEHAVIOUR_ANIMS_BETWEEN_NOSE_CHANGES;
			}

		} else {
			//We've been magnetically pacified so sleep
			if(DEBUG) {
				Serial.begin(BAUD_RATE);
				DEBUGp("Magnetically pacified.");
			}
			power_sleep_long(1);
		}
	}
}


void playHappyAnim(int select) {
	if(select == -1) select = random(0, 17);

	switch(select){
	case 0: {Twinkle a(NULL); play_animation(&a, 2000);} break;
	case 1: {FiveLights a(close5); play_animation(&a, -1);} break;
	case 2: {Pulse a; play_animation(&a, -1);} break;
	case 3: {BeatIndicator a(misland); play_animation(&a, -1);} break;
	case 4: {ScanFade a; play_animation(&a, 1000);} break;
	case 5: {Circle a(NULL); play_animation(&a, -1);} break;
	case 6: {Twinkle a(triad); play_animation(&a, 1000);} break;
	case 7: {TinyFanfare a; play_animation(&a, -1);} break;
	case 8: {Scatman a; play_animation(&a, -1);} break;
	case 9: {Triple a; play_animation(&a, -1);} break;
	case 10: {Cobo a; play_animation(&a, -1);} break;
	case 11: {MOTD a; play_animation(&a, -1);} break;
	case 12: {SMT a; play_animation(&a, -1);} break;
	case 13: {Tetris a; play_animation(&a, -1);} break;
	case 14: {Intel a; play_animation(&a, -1);} break;
	case 15: {Chirp a; play_animation(&a, -1);} break;
	case 16: {HCock a; play_animation(&a, -1);} break;

	default: {Twinkle tw(NULL); play_animation(&tw, 2000);} break;
	}
}
