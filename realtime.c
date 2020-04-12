/*
 * realtime.c
 *
 *  Created on: Nov 20, 2010 (32 kHz, ATmega128)
 *  Revised: July 7, 2012 (add generic 16 MHz, ATmega168 support)
 *  Author: Marc Verdiell
 *
 *	Multitasking and non blocking timer support.
 *
 *  Sets up an interrupt service every 1/100th (or 1/32th) of a second
 *  Calls realtime_do() every 1/100th (or 1/32th) of a second:
 *  put your background task housekeeping or multitasking dispatch code in this function
 *  Default implementation updates software delay counters and egg timers
 *  Also maintains a clock counter in seconds, minutes and hours
 *  Blinks the heartbeat LED once a second if RT_HEARTBEAT_LED is defined
 *
 *  On ATMega168 with a 16 MHz clock using the 8-bit counter0
 *  calls realtime_do() every 0.01s
 *
 *  This can also be implemented on an AVR ATMega128 using the 8-bit counter0
 *  driven by the 32,768 kHz low frequency crystal. In this case, #define the constant
 *  _USE_32KHZ_ in the header file
 *  will call realtime_do() every 1/32 s
 *
 */

#include <avr/io.h> 			// This will include <avr/iom128.h>
#include <stdint.h> 			// delays
#include <avr/interrupt.h>		// interrupts
#include "toolbox.h"			// clear_bit and set_bit utilities
#include "realtime.h"

// Array of registered functions and timers to call and update at interrupt time
rt_timer* rt_timer_array[RT_MAX_TIMERS];				// array of pointers to timers
rt_do_function rt_function_array[RT_MAX_FUNCTIONS];		// array of functions

// global counters and timers, use them in other files by declaring them as 'extern' variables
// regular counters get incremented every 1/100 (1/32) of a second
// timeout counters get decremented every 1/100 (1/32) of a second until they reach 0 and stay at 0
// rt_seconds get incremented every second
volatile uint16_t rt_count1;		// generic up-counters available for r/w
volatile uint16_t rt_count2;		// counts up to 65536 seconds, 100 (or 32) counts per second

volatile uint16_t rt_seconds;		// seconds counter, read only, overflows every 65536 seconds (18 hours)
volatile uint8_t hundreds;			// clock 1/100 seconds
volatile uint8_t seconds;			// clock seconds
volatile uint8_t minutes;			// clock minutes
volatile uint8_t hours;				// clock hours


/********************************
 *
 * To add and use a timer
 * Define a timer variable
 * 	rt_timer mytimer;
 * Register the address of the timer
 * 	rt_add_timer(&mytimer);
 * Now you can use the timer by setting it to a value, and waiting for it to reach 0
 *  mytimer=1000;
 *  while(mytimer!=0)
 *  {
 *  	// do something for 1000 time units (each unit is 1/100 s)
 *  }
 *
 *  The function returns TRUE if it could add the timer, FALSE if not
 *  (the only reason it can't is because there are too many timers in use)
 *
 *******************************/
bool rt_add_timer(rt_timer* atimer)
{
	uint8_t i;
	for (i=0; i<RT_MAX_TIMERS; i++)
	{
		if(!rt_timer_array[i])
		{
			rt_timer_array[i]=atimer;
			return TRUE;
		}
	}
	return FALSE;
}

/********************************
 *
 * To remove a timer, call
 *
 * 	rt_remove_timer(&mytimer);
 * 	The function returns TRUE if it could and remove the timer, FALSE if not
 *
 *******************************/
bool rt_remove_timer(rt_timer *atimer)
{
	uint8_t i;
	for (i=0; i<RT_MAX_TIMERS; i++)
	{
		if(rt_timer_array[i]== atimer)
		{
			rt_timer_array[i]=0;
			return TRUE;
		}
	}
	return FALSE;
}

/******************************************
 *
 * To add a real time callback background function
 * Make a void function returning void
 * 	void myfunction(void);
 * Call
 *  rt_add_function(myfunction);
 * Your function will be called every 1/100s
 * Returns TRUE if it can add the function
 * FALSE if it can't (too many registered already)
 *
 * ***************************************/

bool rt_add_function(void(*afunction)())
{
	uint8_t i;
	for (i=0; i<RT_MAX_FUNCTIONS; i++)
	{
		if(!rt_function_array[i])
		{
			rt_function_array[i]=afunction;
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************
 *
 * To remove a real time callback function
 * Call
 *  rt_remove_function(myfunction);
 * Returns TRUE if it can find and remove it
 * Make sure the functions are very short, less than 1/100 s!
 *
 * ***************************************/
bool rt_remove_function(void(*afunction)())
{
	uint8_t i;
	for (i=0; i<RT_MAX_FUNCTIONS; i++)
	{
		if(rt_function_array[i]== afunction)
		{
			rt_function_array[i]=0;
			return TRUE;
		}
	}
	return FALSE;
}

/***********************************************************
 *
 * Calls registered background tasks
 * Note that this function can be interrupted by higher priority
 * interrupts: make sure all your 16 bit accesses are atomic,
 * or protect them by placing them in-between cli() and sei() calls.
 *
 * Toggles the real time heartbeat LED
 *
 ***********************************************************/
inline static void realtime_do()
{

// heartbeat LED to blink every second
#ifdef RT_HEARTBEAT_LED
	if (seconds & 0x0001) digitalWrite(RT_LEDPORT, RT_LEDPIN, HIGH);
	else digitalWrite(RT_LEDPORT, RT_LEDPIN, LOW);
#endif

	// callback any registered realtime background function
	uint8_t i;
	for (i=0; i<RT_MAX_FUNCTIONS; i++)
	{
		if(rt_function_array[i])
		{
			rt_function_array[i]();
		}
	}
}

inline static void increment_time()
{
	seconds ++;

	if (seconds==60)
	{
		seconds=0;
		minutes++;
		if (minutes==60)
		{
			minutes=0;
			hours++;
			if (hours==24)
			{
				hours=0;
			}
		}
	}
}



void realtime_init()
{
	/*********************************************
	Sets up 8-bit counter0 for interrupts every 1/32 second

	8 bit counter 0 can be run either on 16Mhz clock or asynchronous 32,768 Hz clock.
	32kHz operation is a bit more complicated since it is asynchronous. But it gives an
	exact divider of rt_seconds.

	Counter 0 can be prescaled 8,32,64,128,256,1024 from TCCR0 register
	Free running it gets 256 counts and generates overflow (TIMER0_OVF_vect)
	Or alternatively could use CTC mode to generate more frequent interrupts (TIMER0_COMP_vect).

	With 32,768 Xtal, The following range looks best:
	32768=2^15
	Prescaler at 8, free running at 256: interrupt every 1/16s
	Prescaler at 8, running to 128 count: interrupts every 1/32s (preferred?)
	Prescaler at 8, running to 64 count: interrupts every 1/64s

	With 16MHz Xtal,
	Prescaler at 1024, free running to 256: interrupts every 1/61 of a sec approx
	A little fast, but no bad, overlaps with fastest async clock mode.
	Or at prescale 128, counting to 125 gives 1 ms. A little fast with RC pulses on?
	At prescale 256, counting to 625 gets 0.01 s. Two counts to 208 plus one count to 209?
		(actually set OCRA to 199 twice then 125?)


	Here is how to setup Timer/Counter 0:

	TCNT0 - Timer/Counter Register, set or read counter value

	OCR0 - Output Compare Register, holds the 8 bit compare value

	ASSR - Asynchronous Status Register, bit 3 need to be set to operated from 32kHz crystal
	also need to wait for the busy flags before writing new values in the corresponding registers
	Bit 7 6 5 4 3 2 1 0
	– – – – AS0 TCN0UB OCR0UB TCR0UB
	Bit 3 – AS0: Asynchronous Timer/Counter0
	When AS0 is written to one, Timer/Counter is clocked from a crystal Oscillator connected to the Timer Oscillator
	1 (TOSC1) pin.
	TCN0UB: Timer/Counter0 Update Busy
	OCR0UB: Output Compare Register0 Update Busy
	Bit 0 – TCR0UB: Timer/Counter Control Register0 Update Busy

	Warning: When switching between asynchronous and synchronous clocking of
	Timer/Counter0, the Timer Registers TCNT0, OCR0, and TCCR0 might be corrupted. A safe
	procedure for switching clock source is:
	1. Disable the Timer/Counter0 interrupts by clearing OCIE0 and TOIE0.
	2. Select clock source by setting AS0 as appropriate.
	3. Write new values to TCNT0, OCR0, and TCCR0.
	4. To switch to asynchronous operation: Wait for TCN0UB, OCR0UB, and TCR0UB.
	5. Clear the Timer/Counter0 interrupt flags.
	6. Enable interrupts, if needed.

	TCCR0 - Timer/Counter Control Register , CS bits need to be set for pre-scaling/ starting counter
	Bits: 7 6 5 4 3 2 1 0
	FOC0 WGM00 COM01 COM00 WGM01 CS02 CS01 CS00

	Timer Clocking (Bit 0-2)
	CS02 CS01 CS00 Description
	0 0 0 No clock source (Timer/Counter stopped)
	0 0 1 clkT0S/(No prescaling)
	0 1 0 clkT0S/8 (From prescaler)
	0 1 1 clkT0S/32 (From prescaler)
	1 0 0 clkT0S/64 (From prescaler)
	1 0 1 clkT0S/128 (From prescaler)
	1 1 0 clkT0S/256 (From prescaler)
	1 1 1 clkT0S/1024 (From prescaler)

	Timer/Counter Mode (Bit 6, 3)
	WGM01 WGM00
	0 0 Normal 0xFF Immediate MAX
	0 1 PWM, Phase Correct 0xFF TOP BOTTOM
	1 0 CTC OCR0 Immediate MAX
	1 1 Fast PWM 0xFF BOTTOM MAX

	Compare Match Output Mode (Bit 4, 5)

	FOC0: Bit 7, Force Output Compare

	TIMSK - Timer/Counter Interrupt Mask Register, bit 0,1
	Bit 7 6 5 4 3 2 1 0
	OCIE2 TOIE2 TICIE1 OCIE1A OCIE1B TOIE1 OCIE0 TOIE0

	Only OCIE0 and TOIE0 are of interest for counter 0
	OCIE0: Timer/Counter0 Output Compare Match Interrupt Enable
	TOIE0: Timer/Counter0 Overflow Interrupt Enable

	TIFR - Timer/Counter Interrupt Flag Register
	Bit 7 6 5 4 3 2 1 0
	OCF2 TOV2 ICF1 OCF1A OCF1B TOV1 OCF0 TOV0

	**********************************************/

// only used if heartbeat LED present
#ifdef RT_HEARTBEAT_LED
	digitalMode(RT_LEDPORT, RT_LEDPIN, OUTPUT);
#endif


#if defined (_USE_32KHZ_) // code for ATmega128 with low freq 32kHz crystal, 1/32 sec. interrupts

	// clear interrupts bits before changing counter to async mode
	clear_bit(TIMSK, OCIE0);
	clear_bit(TIMSK, TOIE0);

	// set clock source to asynchronous 32.768 kHz crystal
	// (this could also be done on an ATmega168 using the Timer2 asynchronous timer)
	set_bit(ASSR, AS0);

	// set pre-scaler to 8, Clear on Compare mode
	TCCR0=0;				// reset all bits
	set_bit(TCCR0, CS01); 	// prescaler to 8 (32768/8=4096 counts/second)
	set_bit(TCCR0, WGM01);	// clear on compare mode, comment out for interrupt on overflow mode

	// set compare value to 128 for 32 interrupts/rt_seconds (4096/128=32)
	// only 16 interrupts per second if running until overflow (256) mode
	OCR0=128;

	// reset counter
	TCNT0=0;

	// wait for all asynchronous register write operations to finish
	// by waiting for the three busy flags to clear
	while( ASSR & (_BV(TCN0UB) | _BV(OCR0UB) | _BV(TCR0UB)) ) {};

	// reset overflow and compare interrupt  flags
	set_bit(TIFR, OCF0);	// interrupts bits are cleared by actually writing a 1 to it...
	set_bit(TIFR, TOV0);

	// enable timing on compare the interrupts
	set_bit(TIMSK, OCIE0);	// for interrupt on Clear on Compare mode

#else // code based on 16 MHz clock, 1/100 sec. interrupts, Timer0 counter

		// clear on compare mode
		set_bit(TCCR0A, WGM01);

		// set compare value to 207 twice and 208 once (208 and 209 counts, gives 625 counts in 0.01 s)
		// counter_phase goes from 0 to 2 to keep track of which count we are doing.
		OCR0A=207;

		// reset counter
		TCNT0=0;

		// enable timing on compare the interrupts
		set_bit(TIMSK0, OCIE0A);	// enable interrupt on Clear on Compare mode

		// set pre-scaler to 256, this will launch the counter
		set_bit(TCCR0B, CS02); 	// prescaler to 256 (16,000,000/256=62,500 counts/second)
#endif

	// interrupts on
	sei();
}

/*******************************************************
 * Real time interrupt
 * Gets called every 1/32 of a second
 * Will increment global counters to be used outside this loop
 * You can set or reset the counters and eggtimers outside this loop
 * (while disabling interrupts for atomic access?)
 * Add your own real time housekeeping tasks here too
 *
 * This interrupt is defined as NOBLOCK so that another time-critical/faster interrupt can interrupt
 * (like a servo timing or RC decoding interrupt) can interrupt during this infrequent one,
 * in case some slow, non-critical task is added to itin realtime_do()
 * (task with a short delay for ex).
 * The slow task should still complete in less than 1/32 of a second for the
 * real timer to keep it's accuracy
 *
 ********************************************************/

#if defined (_USE_32KHZ_) // code for ATmega128 with low freq 32kHz crystal, 1/32 sec. interrupts
ISR(TIMER0_COMP_vect, ISR_NOBLOCK)	// clear on compare mode - 32 interrupts/s
{
	static uint8_t countseconds=0;

	// user timers updated at interrupt time here, but used outside this module
	rt_count1++;
	rt_count2++;
	// you can add your own count-down timers here
	if(rt_ctrlegtimer) rt_ctrlegtimer--;
	if(rt_blinktimer) rt_blinktimer--;

	// iterate on all timer pointers, decrement the non zero ones
	uint8_t i;
	rt_timer* timerpointer;
	for (i=0; i<RT_MAX_TIMERS; i++)
	{
		if(rt_timer_array[i])	// if non zero, it points to a registered timer
		{
			// decrement it until it reaches zero
			timerpointer=rt_timer_array[i];
			if (!*timerpointer) (*timerpointer)--;
		}
	}


	// do not modify the following, used for real time clock
	countseconds++;
	if(countseconds==COUNT_PER_SECOND)
	{
		rt_seconds++;
		countseconds=0;
		// time elapsed counter clock
		increment_time();
	}

	// add your short real time tasks here
	realtime_do();
}

#else
// code for ATmega168 with 16 MHz crystal, 3 interrupts for 1/100 update intervals
// Two counts to 208 and one count to 209 lasts 0.01 sec.
ISR(TIMER0_COMPA_vect)
{
	static uint8_t countseconds=0;
	static uint8_t counter_phase=0;

	// first count twice to 208
	if(counter_phase<=1)
	{
		OCR0A=207;
		counter_phase++;
		return;
	}
	// 3rd time count to 209
	else
	{
		OCR0A=208;
		counter_phase=0;

		// user timers updated at interrupt time here, but used outside this module
		rt_count1++;
		rt_count2++;

		// iterate on all timer pointers, decrement the non zero ones
		uint8_t i;
		rt_timer* timerpointer;
		for (i=0; i<RT_MAX_TIMERS; i++)
		{
			if(rt_timer_array[i])	// if non zero, it points to a registered timer
			{
				// decrement it until it reaches zero
				timerpointer=rt_timer_array[i];
				if (*timerpointer) (*timerpointer)--;
			}
		}

		// do not modify the following, used for real time clock
		hundreds++;
		if(hundreds==100) hundreds=0;
		countseconds++;
		if(countseconds==COUNT_PER_SECOND)
		{
			rt_seconds++;
			countseconds=0;
			// time elapsed counter clock
			increment_time();
		}

		// add your short real time tasks here
		realtime_do();
	}
}
#endif

