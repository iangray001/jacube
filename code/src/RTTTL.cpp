/*
 * RTTTL.cpp
 *
 *  Created on: 2 Jul 2013
 *      Author: iang
 */

#include "animations.h"
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "notes.h"

const prog_uint16_t notes[] PROGMEM = {
		0, NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
		NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
		NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
		NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7,
		2*NOTE_C7, 2*NOTE_CS7, 2*NOTE_D7, 2*NOTE_DS7, 2*NOTE_E7, 2*NOTE_F7, 2*NOTE_FS7, 2*NOTE_G7, 2*NOTE_GS7, 2*NOTE_A7, 2*NOTE_AS7, 2*NOTE_B7, 0};


#define OCTAVE_OFFSET 0
#define isdigit(n) (n >= '0' && n <= '9')

#define STATE_PARSE 0
#define STATE_PLAYING 1
#define STATE_DONE 2


/**
 * Special function for reading data from program memory
 */
inline char read_byte(const char *p, bool pgm) {
	if (pgm) return pgm_read_byte(p);
	else return *p;
}


RTTTL::RTTTL(int pin, const prog_char * const song, Animation *callinganim) {
	this->valid = (song != NULL);
	this->pin = pin;
	this->song = song;
	this->state = STATE_PARSE;
	this->wholenote = 0;
	this->bpm = 63;
	this->default_dur = 4;
	this->default_oct = 6;
	this->noteon = false;
	this->callinganim = callinganim;
}

RTTTL::~RTTTL() {}

int RTTTL::play() {
	int num;
	long duration;
	byte note;
	byte scale;

	//Do we need to parse the start of the RTTTL first?
	if(this->state == STATE_PARSE) {
		// format: d=N,o=N,b=NNN:
		// find the start (skip name, etc)
		while(read_byte(song, true) != ':') song++;    	// ignore name
		song++;                     	// skip ':'

		// get default duration
		if(read_byte(song, true) == 'd') {
			song++; song++;				// skip "d="
			num = 0;
			while(isdigit(read_byte(song, true))) {
				num = (num * 10) + (read_byte(song, true) - '0');
				song++;
			}
			if(num > 0) default_dur = num;
			song++;						// skip comma
		}

		// get default octave
		if(read_byte(song, true) == 'o') {
			song++; song++;              // skip "o="
			num = read_byte(song, true) - '0';
			song++;
			if(num >= 3 && num <=7) default_oct = num;
			song++;                   // skip comma
		}

		// get BPM
		if(read_byte(song, true) == 'b') {
			song++; song++;              // skip "b="
			num = 0;
			while(isdigit(read_byte(song, true))) {
				num = (num * 10) + (read_byte(song, true) - '0');
				song++;
			}
			bpm = num;
			song++;                   // skip colon
		}

		// BPM usually expresses the number of quarter notes per minute
		wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

		this->state = STATE_PLAYING;
	}

	//Do we have a note to turn off?
	if(this->noteon) {
#ifdef ENABLE_PIEZO
		noTone(pin);
#endif
		this->noteon = false;
	}

	//Have we finished playing (i.e. is the song pointer at \0?)
	if(!read_byte(song, true)) {
		this->state = STATE_DONE;
#ifdef ENABLE_PIEZO
		noTone(pin);
#endif
		return -1;
	}

	//Play notes
	if(this->state == STATE_PLAYING) {
		//Get note duration, if available
		num = 0;
		while(isdigit(read_byte(song, true))) {
			num = (num * 10) + (read_byte(song, true) - '0');
			song++;
		}

		if(num) duration = wholenote / num;
		else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

		// now get the note
		note = 0;
		switch(read_byte(song, true)) {
		case 'c': note = 1; break;
		case 'd': note = 3; break;
		case 'e': note = 5; break;
		case 'f': note = 6; break;
		case 'g': note = 8; break;
		case 'a': note = 10; break;
		case 'b': note = 12; break;
		case 'p':
		default:
			note = 0;
		}
		song++;

		//Get optional '#' sharp
		if(read_byte(song, true) == '#') {
			note++;
			song++;
		}

		//Get optional '.' dotted note
		if(read_byte(song, true) == '.') {
			duration += duration/2;
			song++;
		}

		//Get scale
		if(isdigit(read_byte(song, true))) {
			scale = read_byte(song, true) - '0';
			song++;
		} else {
			scale = default_oct;
		}

		scale += OCTAVE_OFFSET;

		if(read_byte(song, true) == ',') song++;       // skip comma for next note (or we may be at the end)

		//Play the note
		if(note) {
#ifdef ENABLE_PIEZO
			noTone(pin);
			tone(pin, pgm_read_word(&notes[(scale - 4) * 12 + note]));
#endif
			noteon = true;
			//Call back into the animation
			callinganim->beat_callback();
			return duration;
		} else {
			//Call back into the animation
			callinganim->beat_callback();
			return duration;
		}
	} else {
		return -1;
	}
}

boolean RTTTL::play_done() {
	return state == STATE_DONE;
}

