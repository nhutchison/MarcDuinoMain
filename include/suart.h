/*
 * suart.c
 * Software serial communication implementation
 *
 * Turns a regular I/O pin into a serial output
 *
 * In order to preserve hardware timers for other uses, this uses calibrated delays
 * For best timing compile with interrupts off (define SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT),
 * but at 9600 bauds with short interrupts it does not matter
 *
 * Created July 7, 2012
 * Author: Marc Verdiell
 * Inspired in part by Arduino NewSoftSerial library
 *
 */

#ifndef suart_h
#define suart_h

#include <avr/pgmspace.h>	// for reading strings from program memory
#include "main.h"			// for the _MARCDUINOV2_ compile flag

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifndef F_CPU
/* prevent compiler error by supplying a 16 MHz default */
# warning "F_CPU not defined for <suart.h>"
# define F_CPU 16000000UL
#endif


// for very high baud rates you might need to uncomment this for precise timing.
// It will turn off interrupts while transmitting on the software UART.
// But at 9600 bauds no need to do this if running interrupt handlers are fast enough

#define SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT


// comment the following line out if you only need one SUART port
#define SUART_DUAL_PORT


//****** first default port **********
// Put your Tx pin location here
// SUART is Slave output
#define SUART_TX_PIN	0
#define SUART_TX_PORT 	PORTC

// just three functions needed for writing
void suart_init(long baudrate);
void suart_putc(uint8_t b);
void suart_puts(char* string);
void suart_puts_p(const char *progmem_s );
void suart_tunedDelay(uint16_t delay);
void suart_tx_pin_write(uint8_t pin_state);

//*********second optional port ******
#ifdef SUART_DUAL_PORT

//Suart 2 is MP3/Teeces Out
	#ifdef _MARCDUINOV2_ // Marcduino V2 board has PC1 as SUART2
		#define SUART2_TX_PIN	1
		#define SUART2_TX_PORT	PORTC
	#else				// Marcduino V1 board has PC4 as SUART2
		#define SUART2_TX_PIN	4
		#define SUART2_TX_PORT	PORTC
	#endif

#endif

//PC2 == AUX1 == Suart3???

void suart2_init(long baudrate);
void suart2_putc(uint8_t b);
void suart2_puts(char* string);
void suart2_puts_p(const char *progmem_s );
void suart2_tx_pin_write(uint8_t pin_state);

#endif
