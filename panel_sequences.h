/*
 * panel_sequences.h
 *
 *  Created on: Aug 5, 2012
 *  Author: Marc Verdiell
 *  Defines the sequences matrices for the dome panel moves
 *  Each of them is stored in program memory
 *  See sequencer.h for details on how to declare and use them
 *  See examples on how they are used in main.c
 *
 *  Revised on March 14th, 2020
 *  	Author: Neil Hutchison
 *
 *  Added Per row speed configuration
 *  Added Ability to select a range of servos to be acted on
 *  Added Partial Panel opening
 *  Added 13 panel support
 *
 */

/************ example of how to use the sequencer
* define a sequence matrix like this
* make sure to follow exactly this declaration to put the array in program memory
* (it needs the const and the PROGMEM):

	sequence_t const servo_dance PROGMEM=
	{
			// time	servo1	servo2	servo3	servo4 first last
			{50, 	1000, 	1000, 	1000, 	1000,   1,     11},
			{50, 	2000, 	1000, 	1000, 	1000,   1,     11},
			{50, 	1000, 	2000, 	1000, 	1000,   1,     11},
			{50, 	1000, 	1000, 	2000, 	1000,   1,     11},
			{50, 	1000, 	1000, 	1000, 	2000,   1,     11},
			{50, 	1000, 	1000, 	1000, 	1000,   1,     11},
			{50, 	2000, 	2000, 	1000, 	1000,   1,     11},
			{50, 	1000, 	1000, 	1000, 	1000,   1,     11},
			{50, 	1000, 	1000, 	2000, 	2000,   1,     11},
			{50, 	1500, 	1500, 	1500, 	1500,   1,     11},
			{0, 	_NP, 	_NP, 	_NP, 	_NP,    1,     11},
	};

	// time units are 1/100 seconds. Max is 65535.
	// 10 is 1/10 of a second
	// 100 is 1 second
	// 1000 is 10 second
	// 6000 is a minute
	// 36000 is 6 minutes

	// optionally define a speed matrix for each servo, (0 = max speed, larger numbers = lower speed)
	int16_t servo_dance_speed[]={50,20,0,20};

	first defines the first servo to be called in the row
	last defines the last servo to be called in the row


	in main() or elsewhere call:

	seq_loadsequence(servo_dance, SEQ_SIZE(servo_dance));		// SEQ_SIZE macro defined in sequencer.h to calculate the sequence length
	seq_loadspeed(servo_dance_speed);							// optional, to set the speed of servos
	seq_startsequence();

	To stop it:
	seq_stopsequence();

	Sequence rows work by setting all the valid servo positions, then setting the timeout.
	This means that the timeout applies to the current row.

**************************************/
#ifndef PANEL_SEQUENCES_H_
#define PANEL_SEQUENCES_H_

#include <avr/pgmspace.h>	// for the sequencer data arrays defined with PROGMEM
#include "sequencer.h"		// servo sequencer

// Dome panel sequences

// pulse values for opening and closing panel servos
#define _OPN 1000 // 0
#define _MID 1750 // Test Value ... 1750 (1.75ms) should be ~135
#define _CLS 2000 // 180




sequence_t const panel_all_open PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,   _CLS,  /*_CLS,	_CLS,*/	_NP,	1,		11},
		{300, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,   _OPN,  /*_OPN,	_OPN,*/	_NP,	1,		11},
		{150, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,   _CLS,  /*_CLS,	_OPN,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP,    _NP,   /*_NP,	_NP,*/	_NP,	1,		11}
};

sequence_t const panel_all_open_long PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{1000, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	/*_OPN,	_OPN,*/	_NP,	1,		11},
		{150, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP,	_NP,    /*_NP,	_CLS,*/	_NP,	1,		11}
};

// Recommend not using this until I can figure it out!
sequence_t const panel_all_open_mid PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{200, 	_MID, 	_MID, 	_MID, 	_MID,	_MID, 	_MID, 	_MID, 	_MID,	_MID, 	_MID, 	_MID, 	/*_MID,	_MID,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP,	_NP,    /*_NP,	_CLS,*/	_NP,	1,		11}
};


sequence_t const panel_wave PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_OPN, _CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
	    //{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS, _OPN,*/	_NP,	0,		0}, //=12*30 // note first = 0, so this is skipped
		//{30, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS, _CLS,*/	_NP,	0,		0}, // note first = 0, so this is skipped
		{60, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 90 accounts for the slave panels
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP,    _NP,    /*_NP,	_CLS,*/	_NP,	1,		11}
};

sequence_t const panel_fast_wave PROGMEM=
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS, _OPN,*/	_NP,	0,		0}, // = 11*15=165
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_OPN, _CLS.*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, //30 accounts for Slave sequences.
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	0,		0},
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	/*_CLS, _OPN,*/	_NP,	0,		0},
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_OPN, _CLS,*/	_NP,	0,		0},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};



sequence_t const panel_open_close_wave PROGMEM=
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_OPN, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_OPN, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_CLS,	_CLS, 	_CLS,	_OPN, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_CLS, 	_CLS,	_OPN, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_CLS,	_OPN, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{80, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN,	_OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN, _OPN,*/	_NP,	1,		11}, //= 9x20 + 80 + 3x20 = 180+80+60 = 320
		{20, 	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_OPN, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_CLS, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_OPN,	_CLS, 	/*_CLS, _OPN,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN,	_CLS, 	/*_CLS, _OPN,*/	_NP,	1,		11}, // Stay open for 180
		//{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_OPN,*/	_NP,	0,		0},
		{40, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_OPN,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};

sequence_t const panel_marching_ants PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //1
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //2
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //3
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //4
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //5
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //6
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //7
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //8
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //9
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //10
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //11
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //12
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //13
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //14
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_OPN, 	/*_CLS,	_OPN,*/	_NP,	1,		11}, //15
		{50, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_OPN,	_CLS,*/	_NP,	1,		11},
		{100, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // Close all
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};

sequence_t const panel_dance PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 4 pie, 1 by one
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 4 side, 1 by one
		{45, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_OPN, 	_OPN, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_OPN, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 4 pies. 2 by 2
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 2 large sides
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // interleaved
		{45, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 2nd interleaved
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, //
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, //
		{45, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_OPN, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_OPN, 	_OPN,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // transition
		{45, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_OPN, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // Good
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS, 	_OPN, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN, 	_CLS, 	_OPN,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{45, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};
sequence_t const panel_init PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{100, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP,	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};

// 6min26sec disco sequence to trigger callback at the right time
sequence_t const panel_long_disco PROGMEM=
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_OPN, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_OPN, _CLS.*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS, _OPN,*/	_NP,	0,		0}, // = 12*15=180
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	0,		0},
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	/*_CLS, _OPN,*/	_NP,	0,		0},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		//{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_OPN, 	/*_OPN, _CLS,*/	_NP,	0,		0},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // account for the slaves
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_OPN,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_OPN, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_OPN, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{15, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11}, // 4.05 seconds was 3.45 with 10 servos
		{36000, _NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11},  // 6 minutes
		{2100, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11},  // 21 seconds
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};

// Maxstang's sequences
sequence_t const panel_bye_bye_wave PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{75, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{20, 	_OPN, 	_OPN, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{75, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};

// Maxstang's sequences
sequence_t const panel_wiggle PROGMEM =
{
		//  ---------------------------------------MASTER--------------------------------------------  ----SLAVE------ -----------CONFIG-----------
		// time	servo1	servo2	servo3	servo4	servo5	servo6	servo7	servo8	servo9 servo10 servo11 servo12 servo13 speed	first 	last
		{20, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{50, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN,	_OPN,*/	_NP,	1,		11},
		{14, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{14, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN,	_OPN,*/	_NP,	1,		11},
		{14, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{14, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN,	_OPN,*/	_NP,	1,		11},
		{14, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{14, 	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN, 	_OPN, 	_OPN,	_OPN, 	_OPN,	_OPN, 	/*_OPN,	_OPN,*/	_NP,	1,		11},
		{50, 	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS, 	_CLS, 	_CLS,	_CLS, 	_CLS,	_CLS, 	/*_CLS,	_CLS,*/	_NP,	1,		11},
		{0, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP, 	_NP, 	_NP, 	_NP,	_NP, 	_NP,	/*_NP,	_CLS,*/	_NP,	1,		11}
};
int16_t panel_fast_speed[]={0,0,0,0,0,0,0,0,0,0,0,0};

int16_t panel_medium_speed[]={25,25,25,25,25,25,25,25,25,25,25,25};

int16_t panel_slow_speed[]={15,15,15,15,15,15,15,15,15,15,15,15};

int16_t panel_super_slow_speed[]={9,9,9,9,9,9,9,9,9,9,9,9};


#endif /* PANEL_SEQUENCES_H_ */
