/*
 * servo.h
 *
 *  Created on: Nov 12, 2010
 *      Author: Marc Verdiell
 *
 *      6 independent servo control on AVR-128
 *      Assumes 16 MHz clock
 *      Uses timer counter 1 to count at 0.5 us
 *      This time cannot be used for anything else
 *
 *      Usage:
 *      Set servo port and pins below in the #define section, then:
 *      servo_init();	// need to call first to initialize module. Servo will still be off.
 *      servo_start();	// need to call to start generating servo pulses and turn servos on.
 *      servo_set( servo number (between 1 and SERVO_NUM), value between 500 and 2500);
 *      servo_set( servo number, -1); // will stop that particular servo
 *      position = servo_read(servo number) // will
 *      servo_stop(); //will stop all servo output
 *
 *		Revised: July 12, 2012
 *		Ported to ATmega 168
 *		Any pin / any port for servos, 12 servos max
 *		Support extended range of pulse 500 to 2500
 *		Interface changed to take total pulse length in us, from 500 to 2500
 *
 *	 	Revised: July 18, 2012
 *		Support for one RC input read on ATmega168
 *
 *		*Revised: Sept 30, 2015
 *		Added preliminary support for servo reversing
 *		Right now all servos reversed. I'll need to add 1:1 reversing with EEPROM control later
 *		(#define _REVERSE_SERVOS_ if you want them reversed)
 *
 *		*Revised: March 14,2020
 *		Added support for storing servo direction in EEPROM
 *		Support for per servo reverse added
 *		Currently stored in a word at address 0
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>

// comment out if you do not want to use the ICP pin for the RC input
#define SERVO_RCINPUT

// Servo direction is set in the MarcDuino Setup functions
// This is stored in the EEPROM and read at startup
//uint8_t servo_direction;
uint8_t servo_direction[12];		// per servo direction
uint8_t last_servo;					// Used to limit panel sequences if using less servos.

// compatibility with ATmega48/88/168
#  if !defined(TIMSK)
#  define TIMSK TIMSK1
#  endif
#  if !defined(TIFR)
#  define TIFR TIFR1
#  endif

// input pin definition
#ifdef SERVO_RCINPUT

	#define SERVO_RC_TIMEOUT_MAX 10 // number of bad readings before calling it a time-out


	#if defined (__AVR_ATtiny2313__)
		#define SERVO_INPUT_PIN PD6
		#define SERVO_INPUT_PORT PORTD
	#else //defined (__AVR_ATmega168__)
		#define SERVO_INPUT_PIN PB0
		#define SERVO_INPUT_PORT PORTB
//	#else
//		#error This servo.c version only defines input pins on ATmega168 and Attiny3213
	#endif

#endif

#define SERVO_NUM	11		// number of servos, max supported is 12, entries above max servo number
							// are ignored

#define SERVO1_PIN	2		// pins they are attached to (note, can't use PB0 if RC input is on)
#define SERVO2_PIN	3
#define SERVO3_PIN	4
#define SERVO4_PIN	5
#define SERVO5_PIN	6
#define SERVO6_PIN	1
#define SERVO7_PIN	2
#define SERVO8_PIN	3
#define SERVO9_PIN	4
#define SERVO10_PIN	5
#define SERVO11_PIN	7      // should correspond to pin 13 == AUX 2 on MDv1.5
#define SERVO12_PIN	12
#define SERVO1_PORT PORTD  // ports they are attached to
#define SERVO2_PORT PORTD
#define SERVO3_PORT PORTD
#define SERVO4_PORT PORTD
#define SERVO5_PORT PORTD
#define SERVO6_PORT PORTB
#define SERVO7_PORT PORTB
#define SERVO8_PORT PORTB
#define SERVO9_PORT PORTB
#define SERVO10_PORT PORTB
#define SERVO11_PORT PORTD // PD7
#define SERVO12_PORT PORTB

#define SERVO_PULSE_MIN		500 	// extended range for 180 deg movement
#define SERVO_PULSE_MAX		2500
#define SERVO_PULSE_CENTER	1500

// pause between servo pulse trains in 0.5 us increments
#ifdef SERVO_RCINPUT
	//#define SERVO_PULSE_PAUSE -17536	// make it 1.5x16 ms (65536-17536/2=24ms) if reading pulses
	#define SERVO_PULSE_PAUSE -20000	// a little more responsive for 10 servos
	#else								// so we can be sure to read at least pulse during the pause time
	#define SERVO_PULSE_PAUSE 32000		// if not, regular pause is 16 ms
#endif

#define SERVO_NO_PULSE -1			// pass -1 for no pulse output on that particular servo

// init, start and stop the servo pulses. Init calls start automatically,
// Only call start if you have stopped. Stopping will also stop
// reading the RC input
void servo_init();
void servo_start();
void servo_stop();


/*****************************************************
 * sets servo position value
 * Valid servo are 1 to SERVO_NUM (max is currently 12)
 * Acceptable positions values are SERVO_PULSE_MIN_EX to SERVO_PULSE_MAX_EX.
 * Values lesser or more than these will be clipped to the min or max.
 * If set at SERVO_NO_PULSE or negative, no pulse is generated on that servo output
 * Standard servo pulses vary from 1000 us to 2000us
 * Servo that can turn 180 deg. accept pulses from 500 us to 2500 us
 * Center is at 1500 us, which is SERVO_PULSE_CENTER
 * Values are stored as double that in counter tick values
 * (each counter tick is 0.5 us)
 * in the global array servo_value[]
 *****************************************************/
void servo_set(uint8_t servo, int16_t value);
int16_t servo_read(uint8_t servo);


#ifdef SERVO_RCINPUT
/*****************************
* Only if compiled when SERVO_RCINPUT switch defined above
* This will return the value of the pulse read on ICP or ICP1 pin
* ****************************/
int16_t servo_RCread();

// private functions
void servo_doRCread_init();
void servo_doRCread_start();
void servo_doRCread_end();

#endif

#endif /* SERVO_H_ */
