/***********************************************************
 *  MP3sound.c
 *	MarcDuino interface to play sounds from an MP3Trigger board
 *
 *  Created on: Sep 17, 2013
 *  Author: Marc Verdiell
 *  Copyright � 2013 Marc Verdiell, All Rights Reserved
 *
 ***********************************************************/

/***********************************************************
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
 *  The pre-made R2 sound library contains only a few non-copyrighted music sounds.
 *  Sound 202, 203 and 205 are "beeped music" place-holders, meant to be replaced with the
 *  original score of Star Wars, Empire March, and Cantina respectively
 *
 *  If you add your own sounds in Bank 9, make sure you update the last variable
 *  MP3_BANK9_SOUNDS below to reflect the total number of sounds
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
#include "MP3sound.h"
#include <util/delay.h>		// _delay_ms
#include <avr/pgmspace.h>	// for PROGMEM program space strings

#include <stdlib.h>			// for itoa(), integer to string conversion
#include <stdio.h>			// for sprintf(), don't use if you are short on memory
#include <string.h>			// for strlen()
#include <ctype.h>			// for isdigit()
#include <avr/interrupt.h>	// for sei() in our reinterruptible interrupt routine

#include "toolbox.h"
#include "realtime.h"		// real time interrupt services
#include "serial.h"			// hardware serial
#include "suart.h"			// software serial (write only)
#include "wmath.h"			// random

#ifdef _MP3_DEBUG_MESSAGES_
#include "Print.h"			// quick and easy debug printing
#endif

// global variables, current indexes to banks
static uint8_t mp3_bank_indexes[MP3_MAX_BANKS];
static const uint8_t mp3_max_sounds[] =
		{
		 MP3_BANK1_SOUNDS,
		 MP3_BANK2_SOUNDS,
		 MP3_BANK3_SOUNDS,
		 MP3_BANK4_SOUNDS,
		 MP3_BANK5_SOUNDS,
		 MP3_BANK6_SOUNDS,
		 MP3_BANK7_SOUNDS,
		 MP3_BANK8_SOUNDS,
		 MP3_BANK9_SOUNDS,
		};
static uint8_t mp3_volume=MP3_VOLUME_MID;
static uint8_t saveflag;
const char strSoundCmdError[] PROGMEM = "Invalid MP3Trigger Sound Command";
static uint8_t soundUsed = 0; // Defaults to SparkFun MP3 Trigger.  Updated in the init function.

// globals needed for random sound function in main loop
rt_timer mp3_random_timer;	// the timer to the next random sound
uint8_t mp3_random_mode_flag=0;	// the switch to check random sound mode

/******* this function needs to be called in the main loop for random sounds to work ******/
void mp3_do_random()
{
	if(!mp3_random_mode_flag)return;
	if(mp3_random_timer) return;
	if(mp3_random_timer==0)
	{
		// play a random sound
		mp3_random();
		// set the timer for next sound
		mp3_random_timer=random_howsmall_howbig(MP3_MIN_RANDOM_PAUSE, MP3_MAX_RANDOM_PAUSE);
	}
}


void mp3_start_random()
{
	mp3_random_timer=0;
	mp3_random_mode_flag=1;
}

void mp3_stop_random()
{
	mp3_random_mode_flag=0;
	mp3_random_timer=0;
}

void mp3_suspend_random()
{
	mp3_random_timer=MP3_MAX_PAUSE_ON_RESUME;
	saveflag=mp3_random_mode_flag;
	mp3_random_mode_flag=0;
}

void mp3_resume_random()
{
	mp3_random_mode_flag=saveflag;
}



// you need to wait 3 or 4 seconds after power up to call that one
void  mp3_init(uint8_t player_select)
{
	// this implementation assumes the realtime unit has already been initialized in main.c
	// rt_init();
	// and we also assume suart2 has been initialized in main.c
	// suart2_init(9600);

	// Allows us to configure between the SparkFun Mp3 trigger (0) and the DF Player Mini (1)
	soundUsed = player_select;

	// If the DFPlayer is selected, we need to update the sound stuff.
	if (soundUsed) {
		mp3_volume=DF_VOLUME_MID;
		// Call all the setup stuff for the EQ etc.
		// Set the Equaliser to Normal
		uint8_t setup_cmd [] = { 0x7E, 0xFF, 0x06, 0x07, 00, 00, 00, 0xFE, 0xED, 0xEF};
		sendDFP(setup_cmd);
	}


	for(uint8_t i=0; i<MP3_MAX_BANKS; i++)
	{
		mp3_bank_indexes[i]=0;
	}

	// set volume somewhere in the middle
	mp3_volumemid();

	if (mp3_start_sound != 0)
	{
		// play the startup sound (this function return immediately)
		mp3_playstartsound();
	}

	// register our random timer with the realtime.c unit
	rt_add_timer(&mp3_random_timer);
}

void mp3_parse_command(char* commandstr)
{
	////////////////////////////////////////////////
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


	uint8_t len=strlen(commandstr);
	// check the start character
	if(commandstr[0]!=SOUND_START_CHAR)
	{
		serial_puts_p(strSoundCmdError);
		return;
	}

	// should have between 2 and 4 characters
	if (len<2 || len>4)
	{
		serial_puts_p(strSoundCmdError);
		return;
	}

	char cmdch=commandstr[1];

	// if the command character is a digit, this is a sound play command
	if(isdigit(cmdch))
	{
		mp3_stop_random(); // any manual sound command stops random automatically
		uint8_t bank=(uint8_t)cmdch - 48; // cheap ASCII to number conversion
		uint8_t sound=0;
		if(len>2)
		{
			sound=atoi(commandstr+2);
		}
		mp3_sound(bank, sound);
		return;
	}

	// the command is a character
	switch(cmdch)
	{
		case 'R':	// R - random from 4 first banks
			mp3_start_random();	// keep firing random sounds
			//mp3_random();		// this is just a one shot
			break;
		case 'O':	// O - sound off
			mp3_stop_random();
			mp3_volumeoff();
			break;
		case 'L':	// L - Leia message (bank 7 sound 1)
			//mp3_stop_random();		// so long (34s), just stop random?
			mp3_suspend_random();
			mp3_sound(7,1);
			mp3_random_timer=4400; // 34s + 10s extra long delay
			mp3_resume_random();
			break;
		case 'C':	// C - Cantina music (bank 9 sound 5)
			//mp3_stop_random();		// so long, just stop random
			mp3_suspend_random();
			mp3_sound(8,5);
			mp3_random_timer=5600; // extra long delay
			mp3_resume_random();
			break;
		case 'c':	// c - Beep cantina (bank 9 sound 1)
			mp3_suspend_random();
			mp3_sound(8,1);
			mp3_random_timer=2700; // extra long delay
			mp3_resume_random();
			break;
		case 'S':	// S - Scream (bank 6 sound 1)
			mp3_suspend_random();
			mp3_sound(6,1);
			mp3_resume_random();
			break;
		case 'F':	// F - Faint/Short Circuit (bank 6 sound 3)
			mp3_suspend_random();
			mp3_sound(6,3);
			mp3_resume_random();
			break;
		case 'D':	// D - Disco (bank 9 sound 6)
			mp3_suspend_random();
			mp3_sound(8,6);
			mp3_random_timer=39600; // 6:26 +10s min extra long delay
			mp3_resume_random();
			break;
		case 's':	// s - stop sounds
			mp3_stop_random();
			mp3_stop();
			break;
		case '+':	// + - volume up
			mp3_volumeup();
			break;
		case '-':	// - - volume down
			mp3_volumedown();
			break;
		case 'm':	// m - volume mid
			mp3_volumemid();
			break;
		case 'f':	// f - volume max
			mp3_volumemax();
			break;
		case 'p':	// p - volume min
			mp3_volumemin();
			break;
		case 'W':	// W - Star Wars music (bank 9 sound 2)
			mp3_stop_random();		// so long, just stop random
			//mp3_suspend_random();
			mp3_sound(8,2);
			//mp3_resume_random();
			break;
		case 'M':	// M - Imperial March (bank 9 sound 3)
			mp3_stop_random();		// so long, just stop random
			//mp3_suspend_random();
			mp3_sound(8,3);
			//mp3_resume_random();
			break;
		default:
			serial_puts_p(strSoundCmdError);
			break;
	}
}

// Sends the hardware command, assumes MP3 is connected to suart2 on the MarcDuino
void mp3_send_command_byte(char command)
{
	// sends a single byte at a time
	suart2_putc(command);
}

void mp3_random()
{
	uint8_t num;
	// Plays a random sound from the first 5 banks only
	num = random_howsmall_howbig(1,MP3_BANK1_SOUNDS+MP3_BANK2_SOUNDS+ MP3_BANK3_SOUNDS+MP3_BANK4_SOUNDS+MP3_BANK5_SOUNDS);
	if(num<=MP3_BANK1_SOUNDS)
	{
		mp3_sound(1, num);
		return;
	}
	num-=MP3_BANK1_SOUNDS;
	if(num<=MP3_BANK2_SOUNDS)
	{
		mp3_sound(2, num);
		return;
	}
	num-=MP3_BANK2_SOUNDS;
	if(num<=MP3_BANK3_SOUNDS)
	{
		mp3_sound(3, num);
		return;
	}
	num-=MP3_BANK3_SOUNDS;
	if(num<=MP3_BANK4_SOUNDS)
	{
		mp3_sound(4, num);
		return;
	}
	num-=MP3_BANK4_SOUNDS;
	if(num<=MP3_BANK5_SOUNDS)
	{
		mp3_sound(5, num);
		return;
	}
}

// play sound from bank. If sound=0, plays next sound in bank
// for bank 1 to 4, and first sound in bank for bank 5-9
// Bank 0 can access any sound
void mp3_sound(uint8_t bank, uint8_t sound)
{
	uint8_t filenum;

	if(bank>MP3_MAX_BANKS) return;
	if(bank!=0 && sound>MP3_MAX_SOUNDS_PER_BANK) return;

	// if bank=0 play the sound number provided
	if(bank==0) filenum=sound;

	else if(sound!=0)
	{
		// calculate actual file number on the MP3 memory card
		filenum = (bank-1)*MP3_MAX_SOUNDS_PER_BANK + sound;
		// also adjust last sound played index for the next sound command
		// make sure not to go past max sounds
		if(sound>mp3_max_sounds[bank])
			mp3_bank_indexes[bank]= mp3_max_sounds[bank];
		else
			mp3_bank_indexes[bank]= sound;
	}
	// sound "0", play first or next sound depending on bank
	else
	{
		if(bank<=MP3_BANK_CUTOFF)
		{
			// advance index, rewind to first sound if at end
			if((++mp3_bank_indexes[bank]) > mp3_max_sounds[bank])
				mp3_bank_indexes[bank]=1;
			// we'll play the new indexed sound
			sound=mp3_bank_indexes[bank];
		}
		else
		{
			// for banks that always play the first sound
			sound=1;
		}
		filenum = (bank-1)*MP3_MAX_SOUNDS_PER_BANK + sound;
	}

	if (soundUsed){
		playDFP(filenum);
	} else {
		// send a 't'nnn number where nnn=file number
		mp3_send_command_byte(MP3_PLAY_CMD);
		mp3_send_command_byte(filenum);
	}

#ifdef _MP3_DEBUG_MESSAGES_
	printstr("MP3 Sound: ");
	printchar(MP3_PLAY_CMD);
	printchar(' ');
	printlnuint(filenum);
#endif
}

void mp3_playstartsound()
{
	// access the start sound directly through the global bank 0
	mp3_sound(0,mp3_start_sound);
}

void mp3_stop()
{
	// this doesn't work as this is a start/stop combined
	//mp3_send_command_byte(MP3_STOP_CMD);
	// instead go to an empty sound
	if (soundUsed){
		stopDFP();
	} else {
		mp3_sound(0,MP3_EMPTY_SOUND);
	}
//#ifdef _MP3_DEBUG_MESSAGES_
//	printstr("MP3 stop: ");
//	printlnchar(MP3_STOP_CMD);
//#endif
}

void mp3_volumeup()
{
	if (soundUsed){
		uint8_t step=DF_VOLUME_MAX/DF_VOLUME_STEPS;

		// volume was at max or too high
		if(mp3_volume>=DF_VOLUME_MAX) mp3_volume=DF_VOLUME_MAX;
		else
		{
			// the step would be too big, peg to maximum
			if (mp3_volume+step>DF_VOLUME_MAX)
				mp3_volume=DF_VOLUME_MAX;
			// go up down step (volume goes inverse with value)
			else
				mp3_volume+=step;
		}

		mp3_setvolumeDFP(mp3_volume);
	} else {

		uint8_t step=(MP3_VOLUME_MIN - MP3_VOLUME_MAX)/MP3_VOLUME_STEPS;

		// volume was at max or too high
		if(mp3_volume<=MP3_VOLUME_MAX) mp3_volume=MP3_VOLUME_MAX;
		else
		{
			// the step would be too big, peg to maximum
			if (mp3_volume-MP3_VOLUME_MAX<step)
				mp3_volume=MP3_VOLUME_MAX;
			// go up down step (volume goes inverse with value)
			else
				mp3_volume-=step;
		}

		mp3_setvolume(mp3_volume);
	}

}

void mp3_volumedown()
{
	if (soundUsed){
		uint8_t step=DF_VOLUME_MAX/DF_VOLUME_STEPS;
		// volume was set to off, or ended up too low
		if(mp3_volume<DF_VOLUME_MIN) mp3_volume=DF_VOLUME_MIN;
		else
		{
			// the step would be too bit, peg to minimum
			if (mp3_volume - step < DF_VOLUME_MIN)
				mp3_volume=DF_VOLUME_MIN;
			// go up one step (volume goes inverse with value)
			else
				mp3_volume-=step;
		}
		mp3_setvolumeDFP(mp3_volume);
	} else {
		uint8_t step=(MP3_VOLUME_MIN-MP3_VOLUME_MAX)/MP3_VOLUME_STEPS;
		// volume was set to off, or ended up too low
		if(mp3_volume>MP3_VOLUME_MIN) mp3_volume=MP3_VOLUME_MIN;
		else
		{
			// the step would be too bit, peg to minimum
			if (MP3_VOLUME_MIN-mp3_volume<step)
				mp3_volume=MP3_VOLUME_MIN;
			// go up one step (volume goes inverse with value)
			else
				mp3_volume+=step;
		}
		mp3_setvolume(mp3_volume);
	}
}

void mp3_volumemid()
{
	if (soundUsed){
		mp3_volume=DF_VOLUME_MID;
	} else {
		mp3_volume=MP3_VOLUME_MID;
	}
	mp3_setvolume(mp3_volume);
}

void mp3_volumeoff()
{
	if (soundUsed){
		mp3_volume=DF_VOLUME_OFF;
	} else {
		mp3_volume=MP3_VOLUME_OFF;
	}
	mp3_setvolume(mp3_volume);
}

void mp3_volumemax()
{
	if (soundUsed){
		mp3_volume=DF_VOLUME_MAX;
	} else {
		mp3_volume=MP3_VOLUME_MAX;
	}
	mp3_setvolume(mp3_volume);
}

void mp3_volumemin()
{
	if (soundUsed){
		mp3_volume=DF_VOLUME_MIN;
	} else {
		mp3_volume=MP3_VOLUME_MIN;
	}
	mp3_setvolume(mp3_volume);
}

void mp3_setvolume(uint8_t vol)
{
	if (soundUsed){
		mp3_setvolumeDFP(vol);
	} else {
		mp3_send_command_byte(MP3_VOLUME_CMD);
		mp3_send_command_byte(vol);
	}

#ifdef _MP3_DEBUG_MESSAGES_
	printstr("MP3 Volume: ");
	printchar(MP3_VOLUME_CMD);
	printchar(' ');
	printlnuint(vol);
#endif
}

void sendDFP (uint8_t *cmd) {
  uint16_t checksum = 0;
  for (int i=2; i<8; i++) {
    checksum += cmd[i];
  }
  cmd[7] = (uint8_t)~(checksum >> 8);
  cmd[8] = (uint8_t)~(checksum);
  //for (int i=0; i<10; i++) { //send cmd
	  //mp3_send_command_byte(cmd[i]);
	  //suart2_putc(cmd[i]);
  //}

  suart2_putc(cmd[0]);
  suart2_putc(cmd[1]);
  suart2_putc(cmd[2]);
  suart2_putc(cmd[3]);
  suart2_putc(cmd[4]);
  suart2_putc(cmd[5]);
  suart2_putc(cmd[6]);
  suart2_putc(cmd[7]);
  suart2_putc(cmd[8]);
  suart2_putc(cmd[9]);

}
void playDFP(uint16_t index) {
	// {start, version, length, CMD (12 play mp3 folder) 03 - root, feedback (off), param MSB, param LSB, checksum MSB, checksum LSB, end byte}
	static uint8_t play_cmd [10] = { 0x7E, 0xFF, 0x06, 0x12, 00, 00, 00, 0xFE, 0xee, 0xEF};
	play_cmd[5] = (uint8_t)(index >> 8);
	play_cmd[6] = (uint8_t)(index);

	sendDFP(play_cmd);
}

void stopDFP() {
	static uint8_t stop_cmd [] = { 0x7E, 0xFF, 0x06, 0x0E, 00, 00, 00, 0xFE, 0xED, 0xEF};
	sendDFP(stop_cmd);
}

void mp3_setvolumeDFP(uint8_t vol) {
	static uint8_t volset_cmd [] = { 0x7E, 0xFF, 0x06, 0x05, 00, 00, 00, 0xFE, 0xED, 0xEF};
	volset_cmd[6] = (uint8_t)(vol);
	sendDFP(volset_cmd);
}
