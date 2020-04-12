/***********************************************************
 *  MP3sound.h
 *	MarcDuino interface to play sounds from an MP3Trigger board
 *  Created on: Sep 17, 2013
 *  Author: Marc Verdiell
 *  Copyright � 2013 Marc Verdiell, All Rights Reserved
 *
 *  On the MP3, there are a maximum of 255 sound files
 *  They must be named NNN-xxxx.mp3
 *  Where NNN = 001 to 255
 *  The numbering ranges are predetermined, 25 sounds per
 *  bank category
 *       Bank 1: gen sounds, numbered 001 to 025
 *       Bank 2: chat sounds, numbered 026 to 050
 *       Bank 3: happy sounds, numbered 051 to 075
 *       Bank 4: sad sounds, numbered 076 to 100
 *       Bank 5: whistle sounds, numbered 101 to 125
 *       Bank 6: scream sounds, numbered 126 to 150
 *       Bank 7: Leia sounds, numbered 151 to 175
 *       Bank 8: sing sounds (deprecated, not used by R2 Touch)
 *       Bank 9: mus sounds, numbered 201 t0 225
 *
 *  The pre-cooked R2 sound library contains aonly a few non-copyrighted music sounds.
 *  Sound 202, 203 and 205 are beep placeholders, meant to be replaced with the
 *  original score of Star Wars, Empire March, and Cantina respectively
 *
 ***********************************************************/

	/////////////COMMAND VOCABULARY///////////
	// Play sound command by bank/sound numbers
	// $xyy
	// x=bank number
	// yy=sound number. If none, next sound is played in the bank
	//
	// Other commands
	// $c
	// where c is a command character
	// R - random from 4 first banks
	// O - sound off
	// L - Leia message (bank 7 sound 1)
	// C - Cantina music (bank 9 sound 5)
	// c - Beep cantina (bank 9 sound 1)
	// S - Scream (bank 6 sound 1)
	// F - Faint/Short Circuit (bank 6 sound 3)
	// D - Disco (bank 9 sound 6)
	// s - stop sounds
	// + - volume up
	// - - volume down
	// m - volume mid
	// f - volume max
	// p - volume min
	// W - Star Wars music (bank 9 sound 2)
	// M - Imperial March (bank 9 sound 3)
	//
	///////////////////////////////////////////////

#ifndef MP3SOUND_H_
#define MP3SOUND_H_

/////////////////////////////////////////////////////////////////////
// Adjust your total number of music sounds you put on the card here
/////////////////////////////////////////////////////////////////////
#define USER_MUSIC_SOUNDS_NUMBER 5


//////////////// don't change anything below this line //////////////

// #define _MP3_DEBUG_MESSAGES_

#include <stdint.h>	// uint8_t and companions
#include "main.h" 	// for _RELEASEVERSION_ and _PRIVATERELEASE_ switches

#define MP3_MAX_BANKS    			 9	// nine banks
#define MP3_MAX_SOUNDS_PER_BANK     25	// no more than 25 sound in each
#define MP3_BANK_CUTOFF			 	4	// cutoff for banks that play "next" sound on $x

// for the random sounds, needs to know max sounds of first 5 banks
// only important for sounds below cutoff
#define MP3_BANK1_SOUNDS 19	// gen sounds, numbered 001 to 025
#define MP3_BANK2_SOUNDS 18	// chat sounds, numbered 026 to 050
#define MP3_BANK3_SOUNDS 7	// happy sounds, numbered 051 to 075
#define MP3_BANK4_SOUNDS 4	// sad sounds, numbered 076 to 100
#define MP3_BANK5_SOUNDS 3	// whistle sounds, numbered 101 to 125
// unless you change bank cutoff, these are ignored, so I set them to max
#define MP3_BANK6_SOUNDS MP3_MAX_SOUNDS_PER_BANK	// scream sounds, numbered 126 to 150
#define MP3_BANK7_SOUNDS MP3_MAX_SOUNDS_PER_BANK	// Leia sounds, numbered 151 to 175
#define MP3_BANK8_SOUNDS MP3_MAX_SOUNDS_PER_BANK	// sing sounds (deprecated, not used by R2 Touch)
#define MP3_BANK9_SOUNDS MP3_MAX_SOUNDS_PER_BANK // mus sounds, numbered 201 t0 225

// this defines where the startup sound is
#define MP3_EMPTY_SOUND 252	// workaround, used to stop sounds
//#define MP3_START_SOUND 255	// startup sound is number 255
uint8_t mp3_start_sound;  // Startup sound as a global so we can set it

#define MP3_VOLUME_MID 	50	// guessing mid volume 32 is right in-between...
#define MP3_VOLUME_MIN 	100	// doc says anything below 64 is inaudible, not true, 100 is. 82 is another good value
#define MP3_VOLUME_MAX 	0	// doc says max is 0
#define MP3_VOLUME_STEPS 20	// R2 Touch app has 20 steps from min to max
#define MP3_VOLUME_OFF	254 // to turn it off... 255 gets a buzz.

#define MP3_PLAY_CMD 	't'	// command to play sound file on the MP3 trigger
#define MP3_VOLUME_CMD 	'v'	// command to play sound file on the MP3 trigger
#define MP3_STOP_CMD	'O' // command to stop/play  - not used

#define MP3_MIN_RANDOM_PAUSE 600 		// min wait on random sounds
#define MP3_MAX_RANDOM_PAUSE 1000 		// max wait on random sounds
#define MP3_MAX_PAUSE_ON_RESUME 1200 	// default wait to resume random. Works for short sound. Set mp3_random_timer manually for long ones.


// public
void mp3_init();	// wait at least 3s after mp3trigger power up before calling
void mp3_parse_command(char* commandstr);
void mp3_do_random(); // need to be called in the main loop for random sounds to work

// utilities
void mp3_sound(uint8_t bank, uint8_t sound);
void mp3_stop();
void mp3_playstartsound();
void mp3_random();
void mp3_volumeup();
void mp3_volumedown();
void mp3_volumemid();
void mp3_volumeoff();
void mp3_volumemax();
void mp3_volumemin();

// private
void mp3_send_command_byte(char command);
void mp3_setvolume(uint8_t vol);
void mp3_suspend_random();
void mp3_resume_random();
void mp3_stop_random();
void mp3_start_random();
void mp3_check_timer();



#endif /* MP3SOUND_H_ */
