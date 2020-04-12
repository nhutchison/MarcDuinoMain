/*
 * sequencer.c
 *
 *  Created on: Jul 10, 2012
 *      Author: Marc Verdiell
 *
 *  Revised on March 14th, 2020
 *  	Author: Neil Hutchison
 *
 *  	Added Per row speed configuration
 *  	Added Ability to select a range of servos to be acted on
 *  	Added Partial Panel opening
 *  	Added 13 panel support
 */

#include <avr/pgmspace.h> // for reading the sequences from program memory
#include "sequencer.h"
#include "realtime.h"
#include "servo.h"

// sequencer global variables
volatile int16_t seq_current[SERVO_NUM];				// current servo position array
volatile int16_t seq_goal[SERVO_NUM];					// end goal servo position array
rt_timer seq_timeout;									// timer until end of current sequence step
void(*seq_completion_callback)();								// callback function when sequence ends


// local variables
static int16_t servo_speed[SERVO_NUM];
static uint8_t sequence_step;
static uint8_t sequence_started;
static int16_t const (*sequence_array)[SERVO_NUM+SEQUENCE_PARAMETERS]; 	// sequence_array is a pointer to an array of int16_t
														// this pointer should point to program memory
														// we don't copy the array, just equate to the pointer passed
static uint8_t sequence_length;

// initialize by registering our real time callback and our timer
void seq_init()
{
	rt_add_function(seq_dosequence);
	rt_add_timer(&seq_timeout);
}

// pass a void function(void) to this, and it will be called at the end of the sequence
void seq_add_completion_callback(void(*usercallback)())
{
	seq_completion_callback=usercallback;
}

void seq_remove_completion_callback()
{
	seq_completion_callback=0;
}

// servo speed control functions
void seq_loadspeed(speed_t speedarray)
{
	uint8_t i;
	for(i=0; i<SERVO_NUM; i++ )
	{
		servo_speed[i]=speedarray[i];
	}
}

void seq_resetspeed()
{
	uint8_t i;
	for(i=0; i<SERVO_NUM; i++ )
	{
		servo_speed[i]=0;
	}
}

// call this first to load the sequence array
void seq_loadsequence(int16_t const array[][SERVO_NUM+SEQUENCE_PARAMETERS], uint8_t length)
{
	// stop previous sequence right away before changing pointer array
	sequence_started=0;	// that will stop the sequence interrupts calls
	sequence_step=0;	// restart at step one

	// point to the new sequence array and store it's length
	sequence_array=array;
	sequence_length=length;

	// init the servo current position at step 0;
	uint8_t i;
	for (i=1; i<=SERVO_NUM; i++)
	{
		/***** start with current position = first step **/
		//seq_current[i-1]= pgm_read_word(&(array[0][i]));

		// better: we should start from where the real position of the servos are,
		// so if there is a servo speed limit they continue smoothly from there.
		// If the servos were not on (SERVO_NO_PULSE), they'll jump to the start position
		// regardless of servo speed settings.
		seq_current[i-1]= servo_read(i);
		// Also equate goals to current so we start from steady state
		seq_goal[i-1]=seq_current[i-1];
	}
}

// call this second to execute the sequence from the beginning
void seq_startsequence()
{
	sequence_step=0;
	sequence_started=1;
}

// this will restart the sequence from the point where it was stopped
void seq_restartsequence()
{
	sequence_started=1;
}

// this will stop the sequencer
void seq_stopsequence()
{
	sequence_started=0;
	seq_timeout=0;
	if(seq_completion_callback) seq_completion_callback();
}

// call this before calling restart to specify a specific step from which to restart
void seq_jumptostep(uint8_t step)
{
	if(step<sequence_length) sequence_step=step;
}

// internal functions

/*******old implementation, directly set the position of the servos.
 // Use this if you do not need any servo speed control
void seq_setservopos(int16_t array[][SERVO_NUM+1], uint8_t step)
{
	uint8_t i;
	// servo numbering starts at 1 in both the array and the numbering scheme of servo_set
	for (i=1; i<=SERVO_NUM; i++)
	{
		servo_set(i, array[step][i]);
	}
}
******************/

// new version with servo speed control
void seq_setservopos(int16_t const array[][SERVO_NUM+SEQUENCE_PARAMETERS], uint8_t step)
{
	uint8_t i;
	// This doesn't set the servo position directly anymore.
	// It just sets a goal for the the servo to get to.
	// The actual position sent to the servo will move progressively toward the goal
	// as controlled by the speed setting.
	// The servo actual position updates now occurs at interrupt time in seq_dosequence()
	// and are calculated from the servo goal and servo speed values.

	for (i=1; i<=SERVO_NUM; i++)
	{
		// Check to see if we skip this servo
		if (
			   (i < pgm_read_word(&(array[step][START_SERVO_PARAM]))) // First servo to be triggered
			&& (i > pgm_read_word(&(array[step][END_SERVO_PARAM])))   // Last servo to be triggered.  0 will skip the entire row
			)
		{
			// Skip and go to the next servo.
			continue;
		}

		// just udpate the goals, but not the position of the servos directly
		seq_goal[i-1]=pgm_read_word(&(array[step][i]));
		// cutting off servo pulses is the only immediate servo assignment
		if(seq_goal[i-1]==SERVO_NO_PULSE){servo_set(i,SERVO_NO_PULSE);
		// all other servo assignment take place at interrupt time in seq_dosequence()
		}
	}
}

/**********************************************
* This is the real time routine that executes the sequence
* This is called automatically by the realtime.c module every 1/100 of a second
* (see the rt_dorealtime() function in that module).
* Alternately if you do not want to use the realtime.c module, you can call this
* directly every 1/100s using your own timer method
**********************************************/

void seq_dosequence()
{
	// do nothing unless sequencer explicitly started
	if(!sequence_started) return;
	// sequence array pointer not set, return
	if(!sequence_array) return;

	// the first part of this function just updates the servo position
	// towards the goal position at the maximum speed allowed
	uint8_t i;
	int16_t maxspeed;
	int16_t override_max_speed;
	int16_t delta;
	for(i=0; i<SERVO_NUM; i++)
	{
		// See if this sequence row overwrites the global speed setting, and use that instead
		override_max_speed = pgm_read_word(&(sequence_array[sequence_step][SPEED_PARAM]));
		if (override_max_speed != -1)
		{
			maxspeed = override_max_speed;
		}
		else
		{
			maxspeed=servo_speed[i];			// read servo speed
		}

		delta=seq_goal[i]-seq_current[i];	// calculate difference between goal and current
		if (delta==0) continue;				// if goal position already reached, nothing to do

		// speed code 0 means no speed limit
		// SERVO_NO_PULSE means waking up, we have no valid current position info
		// so we must update it instantly too.
		if (maxspeed==0 || seq_current[i]==SERVO_NO_PULSE)
		{
			seq_current[i]=seq_goal[i];		// set current position=goal
			servo_set(i+1, seq_current[i]); // update actual servo position
		}
		else	// get closer to goal maxspeed at a time
		{
			if(delta>0)
			{
				if(delta>maxspeed) seq_current[i]=seq_current[i]+maxspeed;
				else seq_current[i]=seq_goal[i];
			}
			else
			{
				if(delta<maxspeed) seq_current[i]=seq_current[i]-maxspeed;
				else seq_current[i]=seq_goal[i];
			}

			servo_set(i+1, seq_current[i]); // update actual servo position
		}
	}

	// This second part now run the sequence

	if(!seq_timeout==0) return; // wait until previous step has finished

	// step has finished, go to next sequence step
	if (sequence_step<sequence_length-1) // normal step
	{
		seq_setservopos(sequence_array, sequence_step); 					// put servos in position
		seq_timeout=pgm_read_word(&(sequence_array[sequence_step][0]));		// restart timer with step time value
		sequence_step++;													// advance to next step
	}
	else // last step
	{
		// if last step time is zero, means stop

		if(!pgm_read_word(&(sequence_array[sequence_length-1][0])))
		{
			seq_setservopos(sequence_array, sequence_length-1);
			// ### this has a problem, means that the sequence is stopped before the servos
			// actually reach their goal position. The last step is not "performed", except
			// if it's a no pulse (_NP) servo assignment
			sequence_started=0;
			sequence_step=0;
			// call the completion callback
			if(seq_completion_callback) seq_completion_callback();
		}
		else // it's a looping sequence, just rewind sequence step to 0
		{
			seq_setservopos(sequence_array, sequence_length-1);
			seq_timeout=pgm_read_word(&(sequence_array[sequence_length-1][0]));
			sequence_step=0;
		}
	}
}

