/*
 * realtime.h
 *
 *  Created on: Nov 20, 2010
 *  Revised: July 7, 2012 (add generic 16 MHz, ATmega168 support)
 *  Author: Marc Verdiell
 *
 *  Multitasking support. See realtime.c for full description
 *
 */

#ifndef REALTIME_H_
#define REALTIME_H_

#include "toolbox.h"	// for the declaration of bool

// uncomment the following to use asynchronous 32 kHZ low freq crystal on ATmega128
//#define _USE_32KHZ_

#if defined (_USE_32KHZ_)
#define COUNT_PER_SECOND	32 	// assuming 32.768kHz clock, 8-prescaler, CTC to 128
								// counters are incremented/decremented every 1/COUNT_PER_SECOND=1/32 sec.
#else
#define COUNT_PER_SECOND	100 // assuming 16MHz clock, 256-prescaler, CTC twice to 208 and once to 209						// counters are incremented/decremented every 1/COUNT_PER_SECOND= 1/100 sec.
#endif

// heartbeat LED to blink once per second. Undefine RT_HEARTBEAT_LED if you don't use
#define RT_HEARTBEAT_LED
#define RT_LEDPIN  3
#define RT_LEDPORT PORTC

// max capacity for timers and realtime functions
#define RT_MAX_TIMERS 10
#define RT_MAX_FUNCTIONS 3

// a timer is a volatile 16 bit integer
typedef volatile uint16_t rt_timer;
// a realtime function takes void and return void
typedef void(*rt_do_function)();

// global counters, use them in other files by declaring them as 'extern' variables
// regular counters get incremented every 1/100 (1/32) of a second
// timeout counters get decremented every 1/100 (1/32) of a second until they reach 0 and stay at 0
// rt_seconds get incremented every second
extern volatile uint16_t rt_count1;			// counts up to 65536/COUNTS_PER_SECOND (10 min)
extern volatile uint16_t rt_count2;			// counts up to 65536/COUNTS_PER_SECOND (10 min)

extern volatile uint16_t rt_seconds;		// overflows every 65536 rt_seconds (18 hours)
extern volatile uint8_t hundreds;			// clock 1/100 seconds (stops at 99!)
extern volatile uint8_t seconds;			// clock seconds
extern volatile uint8_t minutes;			// clock minutes
extern volatile uint8_t hours;				// clock hours

void realtime_init();

// to add a timer
// declare it like this as a global
// 		rt_timer mytimer;	// declaration as global, needs to be persistent
// register it like this:
//		bool error;
//		error=rt_add_timer(&mytimer);
// use it like this:
//		mytimer=2*COUNT_PER_SECOND;	// for 2 seconds egg timer
// 		if(mytimer==0) 	{ do something, timer has expired }
//		if(mytimer) 	{ do something, timer has not expired }

bool rt_add_timer(rt_timer *atimer);
// to remove a timer
bool rt_remove_timer(rt_timer *atimer);
// to add a real time callback function (should be a void function returning void)
bool rt_add_function(void(*function)());



#endif /* REALTIME_H_ */
