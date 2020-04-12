/***********************************
 * 	toolbox.h
 * 	Utility macros, some inspired by, other expanding upon Arduino wiring
 *  Revised: July 7, 2012 (add generic 16 MHz, ATmega168 support)
 *  Author: Marc Verdiell
 *
 *  Unlike Arduino, these digitalRead/Write/Mode functions can take any pin on any port
 *  	Always use PORTx for port designation (the function will infer DDRx and PINx register address from that)
 *
 */

#ifndef toolbox_h
#define toolbox_h

#include <avr/io.h>
#include <stdint.h> 		// uint8_t and companions
#include <avr/sfr_defs.h> 	// register type definitions and _BV def

// for compatibility with Arduino (wich calls it boolean)
typedef uint8_t bool;
#define TRUE 1
#define FALSE 0

// interrupts on/off
#define interruptsOn() sei()
#define interruptsOff() cli()

// clock rates
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )


// set/clear bit in register, defined in the same vein as <avr/sfr_defs.h>
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) 		// from wiring_private.h
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit)) 			// from wiring_private.h
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) 	// my own
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit)) 		// my own
// sfr_defs.h also contains the similarly defined
// bit_is_set(sfr, bit)
// bit_is_clear(sfr, bit)

// Macro to access DDR (direction) and PIN (input read) registers from their PORT value
// See how these are used in digitalWrite and digitalRead
#define GET_DDR_REG(x) (*(&x - 1))      /* address of data direction register of port x */
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
    /* on ATmega64/128 PINF is on port 0x00 and not 0x60 */
    #define GET_PIN_REG(x) ( &PORTF==&(x) ? _SFR_IO8(0x00) : (*(&x - 2)) )
	#else
	#define GET_PIN_REG(x) (*(&x - 2))    /* address of input register of port x          */
#endif

// set/clear bit in port, same as set_bit and clear_bit above
#define clear_port_pin(port, pin) (port &= ~_BV(pin))
#define set_port_pin(port, pin) (port |= _BV(pin))
#define port_pin_is_set(port, pin) (port & _BV(pin))
#define port_pin_is_clear(port, pin) (!(port & _BV(pin)))

/**********************************
 * digitalWrite pin setting functions (adapted from Arduino syntax)
 * New inline versions, just specify PORTx on which the pin is
 * Port has to be one of PORTx, value is either LOW or HIGH
 *
 * Usage examples:
 *
 * digitalMode(PORTA, 2, INPUT)  // replaces DDRA |= (1 << 2)
 * digitalWrite(PORTA, 2, HIGH)  // replaces PORTA |= (1 << 2)
 * digitalWrite(PORTA, 2, LOW)	 // replaces PORTA &= ~(1 << 2)
 * digitalWrite(PORTA, 2)	 	 // replaces PINA & (1 << 2)
 *
 */

#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1

// Port has to be one of PORTx, value is LOW (0) or HIGH (non-zero)
#define digitalWrite(port, pin, value) ( (value)==LOW ? ((port) &= ~_BV(pin)) : ((port) |= _BV(pin)))
// Port has to be one of PORTx, mode is either INPUT (0) or OUTPUT (non-zero)
#define digitalMode(port,pin,mode) ( (mode)==INPUT ? (GET_DDR_REG(port) &= ~_BV(pin)) : (GET_DDR_REG(port) |= _BV(pin)))
// Port has to be one of PORTx, returns LOW or HIGH
#define digitalRead(port,pin) (port_pin_is_clear(GET_PIN_REG(port),(pin)) ? LOW : HIGH)

// more natural ways of doing casts from wiring_private.h
#define int(x)     ((int)(x))
#define char(x)    ((char)(x))
#define long(x)    ((long)(x))
#define byte(x)    ((uint8_t)(x))
#define float(x)   ((float)(x))
#define boolean(x) ((uint8_t)((x)==0?0:1))

// Quick Lightweight Math
// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// the following define conflicts with the newest math.h libc:s
// #define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
// so comment it out for later versions of the libc
#if __AVR_LIBC_VERSION__ < 10701UL
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#endif

#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))



#endif
