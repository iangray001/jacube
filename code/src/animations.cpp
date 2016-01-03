#include "Arduino.h"
#include "ADXL345.h"
#include "HMC5883L.h"
#include "animations.h"
#include "leds.h"
#include "utils.h"
#include "compass.h"
#include "options.h"
#include "notes.h"
#include <avr/pgmspace.h>

extern ADXL345 accel;
extern HMC5883L compass;

unsigned char brightcols[NUM_BRIGHTCOLS][3] = {
		{LED_SCANMAX, 0, 0},
		{0, LED_SCANMAX, 0},
		{0, 0, LED_SCANMAX},
		{LED_SCANMAX, LED_SCANMAX, 0},
		{LED_SCANMAX, 0, LED_SCANMAX},
		{0, LED_SCANMAX, LED_SCANMAX}
};


//----------------------------------------------------------------------

/*
 * Play the provided animation.
 * Will play for timeout milliseconds, or until the animation considers that
 * it has completed (by returning false from Animation::tick().
 * If timeout is -1 then the animation will play until it completes regardless
 * of time.
 */
void play_animation(Animation *anim, long timeout) {
	unsigned long thistime, timeouttime, lastframetime = 0, lastsensetime = 0;
	unsigned long nextsoundtime = 0;
	long soundrv;
	boolean soundsneeded;

	timeouttime = millis() + timeout; //When will we timeout?

#ifdef USE_LED_SCANNER
	start_led_scanner();
#endif

	enable_sensors();

	soundsneeded = anim->sounds.valid;

	while(true) {
		thistime = millis();

		//Check for timeout
		if(timeout > 0 && thistime >= timeouttime) break;

		//Check to update sensor values
		if(thistime >= lastsensetime + SENSE_MS) {
			AccelerometerScaled acc;
			MagnetometerScaled mag;
			if(accel.ReadScaledAxis(&acc) && compass.ReadScaledAxis(&mag)) {
				getHeading(mag, acc);
			} else {
				//Could not fetch values so discard them for this loop.
			}
			lastsensetime = thistime;
		}

		//Kill any animation if magnetically pacified
		if(magneticallyPacified(compass)) {
			DEBUGln("Magnetically pacified from animation.");
			break;
		}

		//Check for the next graphics frame
		if(thistime >= lastframetime + TICK_MS) {
			//Check for animation complete
			if(!anim->tick()) break;
			lastframetime = thistime;
		}

		//Call the sound engine
		if(soundsneeded && thistime >= nextsoundtime) {
			soundrv = anim->sounds.play();
			if(soundrv != -1) {
				nextsoundtime = soundrv + thistime;
			} else {
				soundsneeded = false;
			}
		}
	}

	disable_sensors();
	clearLEDs();

#ifdef USE_LED_SCANNER
	stop_led_scanner();
#endif

#ifdef ENABLE_PIEZO
	noTone(PIEZO_PIN_1);
#endif

}

//----------------------------------------------------------------------

Animation::Animation(const prog_char * const tune) : sounds(PIEZO_PIN_1, tune, this) {
}

void Animation::beat_callback(void) {
	//Do nothing in the standard implementation
}

Animation::~Animation() {}

//----------------------------------------------------------------------

ScanFade::ScanFade() : Animation(NULL) {
	for(int i = 0; i < 6; i++)
		vals[i] = i;
}

boolean ScanFade::tick() {
	for(int i = 0; i < 6; i++) {
		vals[i]++;
		if(vals[i] > 8) vals[i] = 0;
		setLED(i, vals[i]);
	}

	return true;
}

//----------------------------------------------------------------------

boolean Twinkle::tick() {
	unsigned char tochange = random(1, 3);
	for(unsigned char x = 0; x < tochange; x++) {
		setLED(random(0, NUMLEDS), random(0, LED_SCANMAX - 1), random(0, LED_SCANMAX - 1), random(0, LED_SCANMAX - 1));
	}
	return !this->sounds.play_done();
}

//----------------------------------------------------------------------

Pulse::Pulse() : Animation(NULL) {
	clearLEDs();
	val = 0;
	frame = 0;
	pulsecount = 0;
}

boolean Pulse::tick() {
	unsigned char r, g, b;

	switch(pulsecount) {
	case 0: r = 0; g = 0; b = val; break;
	case 1: r = val; g = 0; b = 0; break;
	default: r = 0; g = val; b = 0; break;
	}

	frame++;
	if(frame == 1) {
		val++;
		if(val >= LED_SCANMAX) {
			val = 0;
			pulsecount++;
		}
		frame = 0;
	}

	for(unsigned char i = 0; i < NUMLEDS; i++) {
		setLED(i, r, g, b);
	}

	if(pulsecount >= 3) return false;
	else return true;
}

//----------------------------------------------------------------------

BearingIndicator::BearingIndicator() : Animation(NULL) {
	frame = 0;
	pulseval = 0;
	flashval = 0;
	flashcount = 0;
}

boolean BearingIndicator::tick() {
	pulseval++;
	flashcount++;
	if(pulseval >= LED_SCANMAX) pulseval = 0;
	if(flashcount >= 1) {
		flashcount = 0;
		if(flashval == 0) {
			flashval = 7;
		} else {
			flashval = 0;
		}
	}

	setLED(getBearingFace(), LED_SCANMAX - 1);
	setLED(oppositeFace(getBearingFace()), 0);

	setLED(clockwise(getBearingFace()), pulseval);
	setLED(anticlockwise(getBearingFace()), pulseval);

	setLED(getTop(), flashval);
	setLED(oppositeFace(getTop()), flashval);

	return true;
}

//----------------------------------------------------------------------

BeatIndicator::BeatIndicator(const prog_char *song) : Animation(song) {

}

boolean BeatIndicator::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void BeatIndicator::beat_callback() {
	for(unsigned char x = 0; x < 5; x++) {
		setLED(random(0, NUMLEDS), random(0, LED_SCANMAX - 1), random(0, LED_SCANMAX - 1), random(0, LED_SCANMAX - 1));
	}
}

//----------------------------------------------------------------------

FiveLights::FiveLights(const prog_char *song) : Animation(song) {
	beatcount = 0;
}

boolean FiveLights::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void FiveLights::beat_callback() {
	clearLEDs();
	if(beatcount == 0) getSides(this->sides);
	if(beatcount == 4) {
		setLED(getTop(), random(0, 8), random(0, 8), random(0, 8));
	} else {
		setLED(sides[beatcount], random(0, 8), random(0, 8), random(0, 8));
	}
	beatcount++;
}

//----------------------------------------------------------------------

Circle::Circle(const prog_char *song) : Animation(song) {
	count = 0;
	state = 0;
}

boolean Circle::tick() {
	if(state == 0 && count == 0) {
		getSides(sides);
	}

	count++;
	if(count >= 3) {
		count = 0;
		state++;
	}

	clearLEDs();
	switch(state) {
	case 0: setLED(sides[0], 8, 8, 8); break;
	case 1: setLED(clockwise(sides[0]), 8, 8, 8); break;
	case 2: setLED(oppositeFace(sides[0]), 8, 8, 8); break;
	case 3: setLED(anticlockwise(sides[0]), 8, 8, 8); break;
	}

	if(state >= 4) return false;
	else return true;
}

//----------------------------------------------------------------------

static unsigned char onbeat = 0;
static unsigned char state = 0;
static unsigned char sides[4];

//----------------------------------------------------------------------

//Tiny fanfare thing

const prog_char tinyFanfareRTTTL[] PROGMEM = "_:d=8,o=6,b=500:c,e,d7,c,e,a#,c,e,a,c,e,g,c,e,a,c,e,a#,c,e,d7";

TinyFanfare::TinyFanfare() : Animation(tinyFanfareRTTTL) {
    onbeat = 0;
}

boolean TinyFanfare::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void TinyFanfare::beat_callback() {
	clearLEDs();
    for (int i=0; i<6; i++) {
        setLED(i, brightcols[onbeat%6][0], brightcols[onbeat%6][1], brightcols[onbeat%6][2]);
        onbeat++;
    }
    onbeat++;
}

//----------------------------------------------------------------------

const prog_char ScatmanRTTTL[] PROGMEM = "_:d=4,o=5,b=250:8b,16b,32p,8b,16b,32p,8b,2d6,16p,16c#.6,16p.,8d6,16p,16c#6,8b,16p,8f#,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8f#,2p,32p,2d6,16p,16c#6,8p,16d.6,16p.,16c#6,16a.,16p.,8e,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8b,16b,32p,8b,16b,32p,8b,2d6,16p,16c#.6,16p.,8d6,16p,16c#6,8b,16p,8f#,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8f#,2p,32p,2d6,16p,16c#6,8p,16d.6,16p.,16c#6,16a.,16p.,8e,2p.,16c#6,8p,16d.6,16p.,16c#6,16a,8p,8e,2p,32p,16f#.6,16p.,16b.,16p.";

Scatman::Scatman() : Animation(ScatmanRTTTL) {
    onbeat = 0;
    getSides(sides);
}

boolean Scatman::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void Scatman::beat_callback() {
	clearLEDs();
    unsigned char onoroff = (onbeat%2==0 ? 0 : 255);
    setLED(getTop(), onoroff, onoroff, onoroff);
    unsigned char overfour = onbeat/4;
    
    for (unsigned char led = 0; led<4; led++) {
        if (onbeat%4 == led) {
            if (overfour%2==0) {
                setLED(sides[led], 255,255,0);
            } else {
                setLED(sides[led], 0,0,255);
            }
        } else {
            if (overfour%2==0) {
                setLED(sides[led], 1,1,0);
            } else {
                setLED(sides[led], 0,0,1);
            }
        }
    }
    
    onbeat++;
}

//----------------------------------------------------------------------

const prog_char tripleRTTTL[] PROGMEM = "_:d=8,o=5,b=400:c,e,g,c,e,g,c,e,g,c6,e6,g6,c6,e6,g6,c6,e6,g6,c7,e7,g7,c7,e7,g7,c7,e7,g7";

Triple::Triple() : Animation(tripleRTTTL) {
    onbeat = 0;
}

boolean Triple::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void Triple::beat_callback() {
	clearLEDs();
    for (int i=0;i<6;i++) {
        if (onbeat%2) {
            setLED(i,255,0,0);
        } else {
            setLED(i,0,0,255);
        }
    }
    onbeat++;
}

//----------------------------------------------------------------------


const prog_char CoboRTTTL[] PROGMEM = "__:d=4,o=5,b=160:32c4,32d4,32e4,32f4,32g4,32a4,32b4,32c,32d,32e,32f,32g,32a,32b,32c6,16d6,8p,8b,8g,8e,8d6,8b,8g,8b,8p,8g,8p,b,8p,8a,8g,16g,16a,8g,8f,8g,p,8f,8g,16g,16b,16d6,16p,16e6,16p,f6,p,8d6,8p,8b,8g,8e,8d6,8b,8g,8b,8p,8g,8p,b,8p,8a,8g,16g,16a,8g,8f,8g,p,8f,8g,16g,16b,16d6,16p,16e6,16p,f6";

Cobo::Cobo() : Animation(CoboRTTTL) {
    onbeat = 0;
}

boolean Cobo::tick() {
	if (onbeat>=16) {
        for (int i=0; i<6; i++) {
            setLED(i, HSV_to_RGB(((onbeat%40*9)+i*60)%360,100,100));
        }
       onbeat++;
    }
	return !this->sounds.play_done();
}

void Cobo::beat_callback() {
	//clearLEDs();
    if (onbeat<16) {
        for (int i=0;i<6;i++) {
            if (onbeat%2) {
                setLED(i,255,255,255);
            } else {
                setLED(i,0,0,0);
            }
        }
        onbeat++;
    }
    
}

//----------------------------------------------------------------------

const prog_char motdRTTTL[] PROGMEM = "_:d=4,o=5,b=200:8c,8f,8a,8c.6,16a,8a,8a,8a,a,8a#,8c.6,16a,8g,8a,8a#,8c,8e,8g,8a#.,16g,8g,8g,8g,g,8a,8a#.,16g,8f,8g,8a,8c,8f,8a,8c.6,16a,8a,8a,8a,a,8a#,8c.6,16a,8a#,8c6,d6,8d6,8e6,8f6,16f6,8e6,16e6,8d6,8f6,8c6,8c6,8d6,8c6,16a#,8a,16a,8g,f";

MOTD::MOTD() : Animation(motdRTTTL) {
    onbeat = 0;
}

boolean MOTD::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void MOTD::beat_callback() {
	clearLEDs();
    if (onbeat%8) {
        for (int i=0;i<6;i++) {
            if (onbeat%2) {
                setLED(i,0,255,0);
            } else {
                setLED(i,0,0,255);
            }
        }
    }
    onbeat++;
}

//----------------------------------------------------------------------

const prog_char mario2RTTTL[] PROGMEM = "_:d=4,o=5,b=200:8g,16c,8e,8g.,16c,8e,16g,16c,16e,16g,8b,a,8p,16c,8g,16c,8e,8g.,16c,8e,16g,16c#,16e,16g,8b,a,8p,16b,8c6,16b,8c6,8a.,16c6,8b,16a,8g,16f#,8g,8e.,16c,8d,16e,8f,16e,8f,8b.4,16e,8d.,c";

SMT::SMT() : Animation(mario2RTTTL) {
}

boolean SMT::tick() {
	clearLEDs();
	return !this->sounds.play_done();
}

void SMT::beat_callback() {
    for (int i=0;i<6;i++) {
        setLED(i,brightcols[i][0],brightcols[i][1],brightcols[i][2]);
    }
}

//----------------------------------------------------------------------

const prog_char happyRTTTL[] PROGMEM = "_:d=4,o=5,b=140:32c4,32d4,32e4,32f4,32g4,32a4,32b4,32c,32d,32e,32f,32g,32a,32b,32c6,32d6,32p,32d6,32p,32d6,32p,d6,a#,c6,16d6,8p,16c6,2d6.";

HappyAnim::HappyAnim() : Animation(happyRTTTL) {
    onbeat = 0;
    state = 0;
}

boolean HappyAnim::tick() {
    if (state==2) {
        
        
        
        for (int i=0;i<6;i++) {
            setLED(i,0,7-(onbeat/25),0);
        }
        
        if (onbeat>=200) {
            clearLEDs();
            state = 3;
        }
        
        onbeat++;
    }
    
	return state!=3;
}

void HappyAnim::beat_callback() {
    if (state == 0) {
        if (onbeat >= 16) {
            state = 1;
        } else {
            for (int i=0;i<6;i++) {
                setLED(i,onbeat/2,0,onbeat/2);
            }
        }
    }
    
    if (state == 1) {
        if (onbeat<24) {
            unsigned char onLED = onbeat-16;
            setLED(onLED,0,255,0);
        } else if (onbeat==24) {
            for (int i=0;i<6;i++) {
                setLED(i,255,0,0);
            }
        } else if (onbeat==26) {
            for (int i=0;i<6;i++) {
                setLED(i,0,0,255);
            }
        } else if (onbeat>=27) {
            for (int i=0;i<6;i++) {
                setLED(i,0,255,0);
                state = 2;
                onbeat = 0;
                return;
            }
        }
    }
    
    onbeat++;
}

//----------------------------------------------------------------------

const prog_char tetrisRTTTL[] PROGMEM = "_:d=4,o=5,b=300:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";

Tetris::Tetris() : Animation(tetrisRTTTL) {
    onbeat = 0;
}

boolean Tetris::tick() {
	state++;
    for (int i=0;i<6;i++) {
        if (state%2) {
            setLED(i,0,0,0);
        } else {
            setLED(i,0,0,255);
        }
    }
	return !this->sounds.play_done();
}

void Tetris::beat_callback() {
}

//----------------------------------------------------------------------

const prog_char intelRTTTL[] PROGMEM = "_:d=16,o=5,b=320:d,p,d,p,d,p,g,p,g,p,g,p,d,p,d,p,d,p,a,p,a,p,a,2p";

Intel::Intel() : Animation(intelRTTTL) {
    onbeat = 0;
}

boolean Intel::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void Intel::beat_callback() {
    if (onbeat<12) {
        for (int i=0;i<6;i++) {
            setLED(i,0,onbeat,onbeat);
        }
    } else {
        int ledVal = 20-onbeat;
        if (ledVal<0) ledVal = 0;
        for (int i=0;i<6;i++) {
            setLED(i,0,ledVal,ledVal);
        }
    }
    onbeat++;
}

//----------------------------------------------------------------------

const prog_char chirpRTTTL[] PROGMEM = "_:d=32,o=4,b=200:e,4p,e,p,e,8p,e,4p,e,8p,e,4p";

Chirp::Chirp() : Animation(chirpRTTTL) {
    onbeat = 0;
}

boolean Chirp::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void Chirp::beat_callback() {
	clearLEDs();
    for (int i=0;i<6;i++) {
        if (onbeat%2) {
            setLED(i,255,255,255);
        } else {
            setLED(i,0,0,0);
        }
    }
    onbeat++;
}


const prog_char hcockRTTTL[] PROGMEM = "_:d=4,o=5,b=200:16c,16p,16f4,8p,8f,32g,32p,16f,32p,16e,32p,16d,32p,16e,8p,16f,32p,16g,8p.,16c,16p,16f4,8p,8f,32g,32p,16f,32p,16e,32p,16d,32p,16e,8p,16f,32p,16g,8p.,16c,16p,16f4,8p,16g#,32p,8c6,16p,16a#,32p,16g#,8p,16c6,32p,8d#6,16p,16c#6,32p,16c6,8p,16d#6,32p,8g6,16p,16f6,32p,16e6,32p,16c#6,32p,16c6,32p,16a#,32p,16g#,32p,16g,32p,8f4";

HCock::HCock() : Animation(hcockRTTTL) {
    onbeat = 0;
}

boolean HCock::tick() {
	//End once the sound is done
	return !this->sounds.play_done();
}

void HCock::beat_callback() {
	clearLEDs();
    setLED(random(0,6),255,255,255);
}

//----------------------------------------------------------------------

Unhappy::Unhappy() : Animation(NULL) {
	unhappiness = 0;
	noseflash = 0;
	noseflash_r = 0;
	noseflash_g = 0;
	noseflash_b = 0;
	statecount = 0;
	state = 0;
	happyreadings = 0;
}

boolean Unhappy::tick() {
	float hue;
	byte r, g, b;

	//Should we be vibrating?
	if(unhappiness > BEHAVIOUR_VIBRATE_THRESHOLD) {
		vibrate_on();
	} else {
		vibrate_off();
	}

	//Check if we've been freaking out too long
	if(unhappiness > BEHAVIOUR_APOPLEXY_THRESHOLD) {
		int apoSleepCycles = 0;
		unhappiness = 0; //I guess he just doesn't love us. Best not get the police called on him.
		happyreadings = 0;
		MagnetometerScaled rawwhensleep;
		compass.ReadScaledAxis(&rawwhensleep);

		for(int i = 0; i < BEHAVIOUR_APOPLEXY_SLEEP_8SECS; i++) {
			MagnetometerScaled newmag;
			power_sleep_long(2);
			enable_sensors();
			refresh_sensors();

			//Has the cube been poked?
			compass.ReadScaledAxis(&newmag);
			if((abs(newmag.XAxis - rawwhensleep.XAxis) > 20) || (abs(newmag.YAxis - rawwhensleep.YAxis) > 20) || (abs(newmag.ZAxis - rawwhensleep.ZAxis) > 20)) {
				start_led_scanner();
				enable_sensors();
				break;
			}

			//Pulse the leds red
			if(apoSleepCycles >= 4) {
				apoSleepCycles = 0;
				start_led_scanner();
				for(int x = 0; x < 2; x++) {
					for(unsigned char i = 0; i < 6; i++) setLED(i, 7, 0, 0);
					delay(300);
					clearLEDs();
					delay(80);
				}
			} else {
				apoSleepCycles++;
			}
		}

		//Back to freaking out then...
	}
	unhappiness++;

	//Lights

	//What state should the pulsing lights be in
	statecount++;
	if(statecount > 2) {
		statecount = 0;
		state++;
		if(state >= 3) {
			state = 0;
		}
	}

	//Colour all the other LEDs on a scale according to unhappiness, hue 170 to 0
	if(unhappiness >= BEHAVIOUR_VIBRATE_THRESHOLD*2) {
		hue = 0.0;
	} else {
		hue = ((float)(BEHAVIOUR_VIBRATE_THRESHOLD*2 - unhappiness)) / (BEHAVIOUR_VIBRATE_THRESHOLD*2) * 170.0;
	}
	HSV_to_RGB(hue, 100.0, 100.0, r, g, b);

	//Which LEDs are on, which are off?
	if(getBearingFace() == -1) {
		//Nose is up or down so all but the nose (which will be set further down)
		for(unsigned char led = 0; led < 6; led++) {
			setLED(led, r, g, b);
		}
	} else {
		//Nose is side on, so use the state to determine
		//First turn all sides off
		clearLEDs();

		switch(state) {
		case 0:
			//Tail only
			setLED(oppositeFace(getNose()), r, g, b);
			break;
		case 1:
			//Top, bottom and sides that aren't the tail or nose
			for(unsigned char led = 0; led < 6; led++) {
				if(led == getNose() || led == oppositeFace(getNose())) {
					//Cleared above
				} else {
					setLED(led, r, g, b);
				}
			}
			break;
		case 2:
			//Only nose, will be set below
			break;
		}
	}

	//Flash the nose on and off
	noseflash++;
	if(noseflash >= 2) {
		noseflash = 0;
		if(noseflash_r == 0 && noseflash_g == 0 && noseflash_b == 0) {
			unsigned char col = random(0, NUM_BRIGHTCOLS);
			noseflash_r = brightcols[col][0];
			noseflash_g = brightcols[col][1];
			noseflash_b = brightcols[col][2];
		} else {
			noseflash_r = 0;
			noseflash_g = 0;
			noseflash_b = 0;
		}
	}
	setLED(getNose(), noseflash_r, noseflash_g, noseflash_b);


	//Sounds
	float percent = ((float) unhappiness / (float)(BEHAVIOUR_VIBRATE_THRESHOLD*3));
	if(percent > 1.0) {
		percent = 1.0;
	}
	//Glissando
#ifdef ENABLE_PIEZO
	noTone(PIEZO_PIN_1);
	tone(PIEZO_PIN_1, (NOTE_C6 - NOTE_A1) * percent + NOTE_A1);
#endif


	//Now check the sensors for if we are correctly oriented yet
	if(pointingCorrectly(BEHAVIOUR_BEARING_LEEWAY)) {
		happyreadings++;
	}

	if(happyreadings >= BEHAVIOUR_HAPPY_READINGS_REQUIRED) {
		//Leave the angry animation
		vibrate_off();
		return false;
	} else {
		return true;
	}
}