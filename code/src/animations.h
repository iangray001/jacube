#ifndef __ANIMATIONS_H
#define __ANIMATIONS_H

#include "Arduino.h"
#include "options.h"
#include <avr/pgmspace.h>

#define NUM_BRIGHTCOLS 6
extern unsigned char brightcols[NUM_BRIGHTCOLS][3];

//----------------------------------------------------------------------

class Animation;

class RTTTL {
public:
	RTTTL(int pin, const prog_char * const song, Animation *callinganim);
	int play();
	virtual ~RTTTL();
	boolean play_done();
	boolean valid;
private:
	Animation *callinganim;
	int pin;
	const prog_char *song;
	unsigned char state;
	unsigned char default_dur;
	unsigned char default_oct;
	int bpm;
	long wholenote;
	boolean noteon;
};

class Animation {
public:
	Animation(const prog_char * const tune);
	virtual ~Animation();
	/*
	 * Called by the main loop to advance to the next tick of the animation.
	 * Each tick will be *approximately* TICK_MS milliseconds long.
	 * This function should return true if there is more animation to come.
	 * False if the animation has completed.
	 */
	virtual boolean tick() = 0;

	RTTTL sounds;

	virtual void beat_callback(void);
};

//----------------------------------------------------------------------


void play_animation(Animation *a, long timeout);


//----------------------------------------------------------------------

class ScanFade : public Animation {
public:
	ScanFade();
	boolean tick();
private:
	unsigned char vals[6];
};

class Twinkle : public Animation {
public:
	Twinkle(const prog_char *song) : Animation(song) {}
	boolean tick();
};

class Pulse : public Animation {
public:
	Pulse();
	boolean tick();
private:
	unsigned char frame;
	unsigned char val;
	unsigned char pulsecount;
};

class BearingIndicator : public Animation {
public:
	BearingIndicator();
	boolean tick();
private:
	unsigned char frame;
	unsigned char pulseval;
	unsigned char flashval;
	unsigned char flashcount;
};

class BeatIndicator : public Animation {
public:
	BeatIndicator(const prog_char *song);
	boolean tick();
	void beat_callback();
};

class FiveLights : public Animation {
public:
	FiveLights(const prog_char *song);
	boolean tick();
	void beat_callback();
private:
	unsigned char beatcount;
	unsigned char sides[4];
};

class Circle : public Animation {
public:
	Circle(const prog_char *song);
	boolean tick();
private:
	unsigned char state;
	unsigned char count;
	unsigned char sides[4];
};

class TinyFanfare : public Animation {
public:
	TinyFanfare();
	boolean tick();
	void beat_callback();
};
 

class Scatman : public Animation {
public:
	Scatman();
	boolean tick();
	void beat_callback();
};


class Triple : public Animation {
public:
	Triple();
	boolean tick();
	void beat_callback();
};

class Cobo : public Animation {
public:
	Cobo();
	boolean tick();
	void beat_callback();
};

class MOTD : public Animation {
public:
	MOTD();
	boolean tick();
	void beat_callback();
};

class SMT : public Animation {
public:
	SMT();
	boolean tick();
	void beat_callback();
};

class HappyAnim : public Animation {
public:
	HappyAnim();
	boolean tick();
	void beat_callback();
};

class Tetris : public Animation {
public:
	Tetris();
	boolean tick();
	void beat_callback();
};

class Intel : public Animation {
public:
	Intel();
	boolean tick();
	void beat_callback();
};

class Chirp : public Animation {
public:
	Chirp();
	boolean tick();
	void beat_callback();
};

class HCock : public Animation {
public:
	HCock();
	boolean tick();
	void beat_callback();
};


class Unhappy : public Animation {
public:
	Unhappy();
	boolean tick();
private:
	int unhappiness;
	unsigned char noseflash;
	unsigned char noseflash_r, noseflash_g, noseflash_b;
	unsigned char statecount;
	unsigned char state;
	unsigned char happyreadings;
};

//----------------------------------------------------------------------

#endif

