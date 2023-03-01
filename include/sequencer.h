/*
 * sequencer.h
 *
 *  Created on: Jul 10, 2012
 *      Author: Marc Verdiell
 *
 *  A servo sequencer. Works in conjunction with servo.c and realtime.c modules
 *  The sequence is held on a matrix, each line representing a "frame" or step
 *  The first element of the frame is the length of the step in 1/100 sec.
 *  Then the position of all servos follows. Currently all servos up to SERVO_NUM
 *  are involved in the sequence, I could change this later.
 *  Use _NP for no pulse if you want the servo powered off, like done at the end here
 *  in the example.
 *  On the last step, a 0 in the time will stop the sequence.
 *  A non zero time will cause a loop back to the first step.
 *
 */


/************example of how to use this sequencer
* define a sequence matrix like this somewhere
* make sure to follow exactly this declaration to put the array in program memory
* (it needs the const and the PROGMEM):

	sequence_t const servo_dance PROGMEM=
	{
			// time	servo1	servo2	servo3	servo4
			{50, 	1000, 	1000, 	1000, 	1000},
			{50, 	2000, 	1000, 	1000, 	1000},
			{50, 	1000, 	2000, 	1000, 	1000},
			{50, 	1000, 	1000, 	2000, 	1000},
			{50, 	1000, 	1000, 	1000, 	2000},
			{50, 	1000, 	1000, 	1000, 	1000},
			{50, 	2000, 	2000, 	1000, 	1000},
			{50, 	1000, 	1000, 	1000, 	1000},
			{50, 	1000, 	1000, 	2000, 	2000},
			{50, 	1500, 	1500, 	1500, 	1500},
			{0, 	_NP, 	_NP, 	_NP, 	_NP},
	};

	// optionally define a max speed for servos, 0 = max speed
	int16_t servo_dance_speed[]={50,20,0,20};


	in main() or elsewhere call:

	seq_loadsequence(servo_dance, SEQ_SIZE(servo_dance));		// use the SEQ_SIZE macro defined above to calculate the sequence length
	seq_loadspeed(servo_dance_speed);							// optional, to set speed of servos
	seq_startsequence();

	To stop it:
	seq_stopsequence();

**************************************/

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include <stdint.h>
#include "servo.h" // for SERVO_NUM


// define your servo frames here
// Frame times in 1/100s

#define _NP SERVO_NO_PULSE
#define SEQ_SIZE(A) (sizeof(A) / sizeof(A[0]))

// Count the number of Non-Servo entries in the sequence
// Means less work if I add more params
#define SEQUENCE_PARAMETERS 4

//Used for indexing into the array
#define SEQUENCE_ROW (SERVO_NUM + SEQUENCE_PARAMETERS)

//Useful stuff
#define SPEED_PARAM SEQUENCE_ROW - 3
#define START_SERVO_PARAM SEQUENCE_ROW - 2
#define END_SERVO_PARAM SEQUENCE_ROW - 1

typedef int16_t sequence_t[][SERVO_NUM + SEQUENCE_PARAMETERS];
typedef int16_t (*sequence_t_ptr)[SERVO_NUM + SEQUENCE_PARAMETERS];
typedef int16_t speed_t[SERVO_NUM];

// public
void seq_init();
void seq_add_completion_callback(void(*usercallback)());
void seq_remove_completion_callback();
void seq_loadspeed(speed_t speedarray);
void seq_resetspeed();
void seq_loadsequence(int16_t const array[][SERVO_NUM + SEQUENCE_PARAMETERS], uint8_t length);
void seq_startsequence();
void seq_stopsequence();
void seq_restartsequence();

// private
void seq_dosequence();
void seq_jumptostep(uint8_t step);
void seq_setservopos(int16_t const array[][SERVO_NUM + SEQUENCE_PARAMETERS], uint8_t step);

#endif /* SEQUENCER_H_ */
