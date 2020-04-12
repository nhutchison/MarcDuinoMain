/*
 * servo.c
 *
 *  Created on: Nov 12, 2010
 *      Author: Marc verdiell
 *
 *
 *      12 independent servo control on ATmega128, ATmega168
 *      Assumes 16 MHz clock
 *      Uses timer counter 1 to count at 0.5 us
 *      This timer cannot be used for anything else
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
 *		Revised: Sept 30, 2015
 *		Added preliminary support for servo reversing
 *		Right now all servos reversed. I'll need to add 1:1 reversing with EEPROM control later
 *		(#define _REVERSE_SERVOS_ if you want them reversed)
 */

#include "servo.h"
#include <avr/interrupt.h>
#include <avr/io.h> 	// This will include <avr/iom128.h>
#include "binary.h" 	// BIT0-BIT31 definitions and binary fields
#include "toolbox.h"




// Macro to access DDR and PIN ports from their PORT value
#define servo_DDR(x) (*(&x - 1))      /* address of data direction register of port x */

// global variables for the servo arrays
uint16_t servo_value[12];			// pulse value in ms, -1 is no pulse
uint8_t servo_pin[12];				// pins to which servos are connected
volatile uint8_t* servo_port[12];	// port addresses to which servos pins are connected

// only if compiled with rc input switch
#ifdef SERVO_RCINPUT
volatile int16_t servo_rcbegin;
volatile int16_t servo_rcend;
volatile uint8_t servo_gotpulse;
volatile uint8_t servo_rctimeout;
volatile uint8_t servo_rcvalid;
volatile int16_t servo_rctemp;
int16_t servo_rcpulse;
#endif

// private global variable updated in interrupt routine
static volatile uint8_t current_servo;

/************************************************
 * Start the servo pulses
 * Sets servo output pins
 * Set counter1 to 0.5 us ticks on 16 MHz clock
 * Enable overflow interrupts
 * Start by counting a long pause
 * *********************************************/
void servo_init()
{

	uint8_t i;

	// servo pulse value array set to no pulse
	for(i=0; i<=11; i++)
	{
		servo_value[i]=SERVO_NO_PULSE; // manipulate directly (could call servo_set() instead)
	}

	// store servo PORTx register addresses in servo_port array
	servo_port[0]=&SERVO1_PORT;
	servo_port[1]=&SERVO2_PORT;
	servo_port[2]=&SERVO3_PORT;
	servo_port[3]=&SERVO4_PORT;
	servo_port[4]=&SERVO5_PORT;
	servo_port[5]=&SERVO6_PORT;
	servo_port[6]=&SERVO7_PORT;
	servo_port[7]=&SERVO8_PORT;
	servo_port[8]=&SERVO9_PORT;
	servo_port[9]=&SERVO10_PORT;
	servo_port[10]=&SERVO11_PORT;
	servo_port[11]=&SERVO12_PORT;

	// store servo pins in servo_pin array
	servo_pin[0]= SERVO1_PIN;
	servo_pin[1]= SERVO2_PIN;
	servo_pin[2]= SERVO3_PIN;
	servo_pin[3]= SERVO4_PIN;
	servo_pin[4]= SERVO5_PIN;
	servo_pin[5]= SERVO6_PIN;
	servo_pin[6]= SERVO7_PIN;
	servo_pin[7]= SERVO8_PIN;
	servo_pin[8]= SERVO9_PIN;
	servo_pin[9]= SERVO10_PIN;
	servo_pin[10]= SERVO11_PIN;
	servo_pin[11]= SERVO12_PIN;


	// Set all servo outputs pins to 0;
	for(i=0; i<SERVO_NUM; i++)
	{
		clear_bit(*servo_port[i], servo_pin[i]);
	}
	// Set all servo pins to output mode
	for(i=0; i<SERVO_NUM; i++)
	{
		set_bit(servo_DDR(*servo_port[i]), servo_pin[i]);
	}
	// Reset all servo values to SERVO_NO_PULSE

	// Timer1 setup, used for timing servo outputs
	TIMSK &= ~_BV(TOIE1); 			// Disable overflow interrupts for now (TIMER1_OVF_vect)
	current_servo = SERVO_NUM;		// start with servo pause
	TCCR1A =0; 						// Simple up counting operation until overflow
	TCNT1= -(SERVO_PULSE_PAUSE);	// start counter at -16000 us

	// further init if RC input is enabled
	#ifdef SERVO_RCINPUT
	servo_doRCread_init();
	#endif


	servo_start(); 					// launch servo unit by default, or input capture would not work
									// since they are all set at SERVO_NO_PULSE it does output pulses

}

/************************************************
 * Start the servo pulses
 * Set counter1 to 0.5 us ticks on 16 MHz clock
 * Enable overflow interrupts
 * Start by counting a long pause
 * *********************************************/
void servo_start()
{
	// Uses clock input divided by 8, 0.5 microsecond/tick
	current_servo = SERVO_NUM;		// start with servo pause
	TCNT1= -(SERVO_PULSE_PAUSE);	// start counter at -16000 us
	TIMSK |= _BV(TOIE1); 			// Enable overflow interrupts
	sei();							// enable global interrupts
	TCCR1B |= _BV(CS11); 			// Launch timer, pre-scaler div clock by 8, gives 0.5 us ticks at 16 MHz
}

/*****************************
 * Stop the servo pulses, and also the input counting
 ****************************/
void servo_stop()
{
	uint8_t i;
	// Set all servo output to 0;
	for(i=0; i<SERVO_NUM; i++)
	{
		clear_bit(*servo_port[i], servo_pin[i]);
	}
	TIMSK &= ~_BV(TOIE1); 	// Disable overflow interrupts
	TCCR1B = 0; 			// Stop timer
}

/*****************************************************
 * sets servo position value
 * Valid servo are 1 to SERVO_NUM (max is currently 12)
 * Acceptable positions values are SERVO_PULSE_MIN to SERVO_PULSE_MAX.
 * Values lesser or more than these will be clipped to the min or max.
 * If set at SERVO_NO_PULSE or negative, no pulse is generated on that servo output
 * Standard servo pulses vary from 1000 us to 2000us
 * Servo that can turn 180 deg. accept pulses from 500 us to 2500 us
 * Center is at 1500 us, which is SERVO_PULSE_CENTER
 * Values are stored as double that in counter tick values
 * (each counter tick is 0.5 us)
 * in the global array servo_value[]
 *****************************************************/
void servo_set(uint8_t servo, int16_t time)
{
	// servo must be 1 to SERVO_NUM
	if(servo==0 || servo>SERVO_NUM) return;

	// time=SERVO_NO_PULSE means no output
	if(time<=SERVO_NO_PULSE)
	{
		servo_value[servo-1]=SERVO_NO_PULSE;
		return;
	}

	// time will be trimmed to (SERVO_PULSE_MAX - SERVO_PULSE_MIN)
	if(time>SERVO_PULSE_MAX) time=SERVO_PULSE_MAX;
	if(time<SERVO_PULSE_MIN) time=SERVO_PULSE_MIN;

	// multiply by two to account for counter ticks of 0.5us
	servo_value[servo-1]=2*time;
}

/*****************************************************
 * gets servo position value
 * Valid servo are 1 to SERVO_NUM (currently 6)
 * Acceptable positions values are 0 to SERVO_PULSE_MAX - SERVO_PULSE_MIN.
 * If set at 0, no pulse is generated on that servo output
 * Internally, pulses vary from 1000 us to 2000us
 * and values are store as counter tick values (double this)
 * in the global array servo_value[]
 *****************************************************/
int16_t servo_read(uint8_t servo)
{
	// servo must be 1 to SERVO_NUM
	if(servo==0 || servo>SERVO_NUM) return 0;

	// time=SERVO_NO_PULSE means no output
	if(servo_value[servo-1]==SERVO_NO_PULSE)
	{
		return SERVO_NO_PULSE;
	}

	// Divide by two to account for 0.5us counter ticks
	// subtract minimum pulse width
	return (servo_value[servo-1]/2);
}




/******************************************************
 * Counter1 Overflow interrupt
 * At each iteration, sets the timer to the
 * next servo pulse length as set in the global variable
 * servo_value[]
 * A value of 0 in the servo means no pulse is put out
 ***************************************************/
ISR(TIMER1_OVF_vect)
{
	// first end the current pulse except if in pause
	if(current_servo>=SERVO_NUM) // we were doing the long pause
	{
		// if RC reading, stop and read the input capture after the long pause
		#ifdef SERVO_RCINPUT
		servo_doRCread_end();
		#endif

		current_servo=0; // restart at first servo
	}
	else // normal case, end the current servo pulse before starting the next one
	{
		clear_bit(*servo_port[current_servo], servo_pin[current_servo]);
		// goto next servo
		current_servo++;
	}

	// now start the next one except if it's time for pause
	if(current_servo>=SERVO_NUM) // we've reached the pause
	{
		TCNT1= -(SERVO_PULSE_PAUSE); // load the counter with long pause value

		// if RC reading, start the input capture during the pause
		#ifdef SERVO_RCINPUT
		servo_doRCread_start();
		#endif
	}
	else	// regular start of a new pulse
	{
		if(servo_value[current_servo] != SERVO_NO_PULSE) // regular pulse value
		{
			// start pulse
			set_bit(*servo_port[current_servo], servo_pin[current_servo]);
			if (servo_direction[current_servo] == 1)
			{
				// set inverse pulse length, and wait for overflow
				// servo values are stored as twice their us value
				TCNT1= servo_value[current_servo]-4*SERVO_PULSE_CENTER;
			}
			else if (servo_direction[current_servo] == 0)
			{

				// set normal pulse length, and wait for overflow
				TCNT1= -(servo_value[current_servo]);
			}

		}
		else	// SERVO_NO_PULSE means no output, wait minimum pulse value
		{
			TCNT1= -SERVO_PULSE_MIN;
		}
	}
}

/***********************************
 * Single channel RC input reading functions
 ***********************************/
#ifdef SERVO_RCINPUT


// this is the user callable function, returns the pulse read in us or
// SERVO_NO_PULSE (-1) if timed out or no valid pulse read for 10 cycles

int16_t servo_RCread()
{
	// we timed out too much, no valid servo reading
	if( servo_rctimeout >= SERVO_RC_TIMEOUT_MAX)
	{
		servo_rcpulse=SERVO_NO_PULSE;	// reset the value to NO_PULSE
		return servo_rcpulse;
	}
	// we got a new clean reading, store it and return it
	if( servo_rctimeout == 0 && servo_rcvalid)
	{
		servo_rcpulse=servo_rctemp/2;
		servo_rcvalid=0;	// we consumed the reading, invalidate it
		return servo_rcpulse;
	}
	// neither new result nor timed out yet, return the last known result
	return servo_rcpulse;
}

// initial setup
void servo_doRCread_init()
{
	// optional, these should be clear anyhow
	clear_bit(ACSR, ACIC); 		// source is ICP pin (set bit for comparator input)
	clear_bit(TCCR1B, ICNC1); 	// no noise cancellation for more precise timing
	clear_bit(TIMSK, ICIE1);	// disable ICP interrupts for now (TIMSK if defined in header as TIMSK1 for ATMega8x)

	// set ICP pin as input
	digitalMode(SERVO_INPUT_PORT, SERVO_INPUT_PIN, INPUT);

	// initial variables, for clarity, should not be needed
	servo_rcbegin=0;
	servo_rcend=0;
	servo_gotpulse=0;
	servo_rctimeout=0;
	servo_rcpulse=SERVO_NO_PULSE;
}

// this is called by the servo interrupt routine on beginning long pause cycle
void servo_doRCread_start()
{
	set_bit(TCCR1B, ICES1); 	// trigger on rising edge
	set_bit(TIFR, ICF1);		// always clear interrupt flag (by setting it to 1) after changing the edge
								// TIFR is defined to TIFR1 for the Atmega168
	servo_gotpulse=0;			// got no pulse yet in this cycle
	set_bit(TIMSK, ICIE1);		// enable input capture interrupts - rest of the work is done by interrupt handler
}

// this is called by the servo interrupt routine on resuming after long pause cycle
void servo_doRCread_end()
{
	//uart0_puts("in RC end \n\r");
	clear_bit(TIMSK, ICIE1);	// disable further ICP interrupts, no more readings until long pause

	// we got a valid pulse reading, store intermediate result.
	if(servo_gotpulse)
	{
		servo_rctimeout=0;						// reset timeout count
		servo_rctemp=servo_rcend-servo_rcbegin;	//hold the intermediate result without further processing
		servo_rcvalid=1;						// flag that there is a valid result in the temporary variable
	}
	// else update the timeout count
	else
	{
		if(servo_rctimeout <= SERVO_RC_TIMEOUT_MAX) servo_rctimeout++;
	}
}



// this is called on interrupt capture event
ISR(TIMER1_CAPT_vect)
{
	cli(); 	// interrupts off, just in case, we want to get this done real fast

	// if we were triggering on the rising edge, this is the pulse beginning
	if (bit_is_set(TCCR1B, ICES1))
	{
		servo_rcbegin=ICR1;			// read the captured begin time
		clear_bit(TCCR1B, ICES1); 	// change trigger to falling edge
		set_bit(TIFR, ICF1);		// always clear interrupt flag (by setting it to 1) after changing the edge
	}
	// this is the falling edge, we successfully timed a pulse
	else
	{
		servo_rcend=ICR1;			// read the captured end time
		servo_gotpulse=1;			// flag that we got a valid pulse reading
		clear_bit(TIMSK, ICIE1);	// disable further ICP interrupts, no more readings needed this cycle
	}

	sei(); 	// interrupts back on

}

#endif
