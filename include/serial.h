/**************************************
 *  serial.h
 *
 *  Created on: Oct 31, 2010
 *  Author: Marc Verdiell
 *
 *  Dual serial UART interface
  *     For devices with one or two UARTS.
 *      Define _HAS_UART1_ if you have a second UART
 *
 *  v1.1 08/19/2013
 *  - added progmem string support on uart0
 *  - auto-define for devices that have two UARTs (for now 128, 1280, 2560)
 *	v2.0 09/13/2013
 *  - switch to interrupt driven, fully buffered on the serial_ (UART1) interface
 *  - second UART is still polled implementation, not updated yet
 *  v2.1 06.01/2015
 *  - made serial_puts wait if output buffer is full
 *  - created serial_puts_nowait if no waiting is required (faster too for fast serial speeds)
 *
 *************************************/

#include <stdint.h>
#include <avr/pgmspace.h>	// for using PROGMEM strings

#ifndef RS232_H_
#define RS232_H_

// for chips that have two UARTs
#if defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define _HAS_UART1_
#endif

// you can change the default ring buffer sizes here
#define BUFSIZE_IN  0x40
#define BUFSIZE_OUT 0xFF

#define PARITYNONE 0
#define PARITYODD 1
#define PARITYEVEN 2

// prevent compiler error by supplying a 16 MHz default
#ifndef F_CPU
# warning "F_CPU not defined for <RS232.h>"
# define F_CPU 16000000UL
#endif

/********* init ***************/
void serial_init(uint16_t baudrate); 	//defaults to 1 stop, no parity.
void serial_init_9600b8N1(void); 		// 9600 bauds, 8 bits, 1 stop, no parity
void serial_init_9600b7E1(void); 		// 9600 bauds, 7 bits, 1 stop, no parity

/********* sending ***********/
uint8_t serial_tx_complete();			// use to check if the output buffer has emptied before sending more if you need flow control
uint8_t serial_putc(unsigned char ch);	// returns 0 if output buffer was full
void serial_puts(char* string);			// send string, waits for output buffer to clear if full
uint8_t serial_puts_nowait(char* string);  // send string, returns immediately, if output buffer is full char will be lost and it will return 0
void serial_puts_p(const char *progmem_s ); // for printing program memory strings
// this is how to use it
// #include <avr/pgmspace.h> // optional, already included in this header
// const char string[] PROGMEM = "Hello World";
// serial_puts_p(string);

/******** receiving **********/
uint8_t serial_available();				// call this first before calling the next one...
unsigned char serial_getc (void);		// returns right away, with 0xFF if no character available
int8_t serial_getc_nowait (void);		// returns -1 if no char, works if expecting 7 bit ASCII
unsigned char serial_getc_wait (void);  // blocks until a character is available

/***** Receive Interrupt Enable **********/
void serial_enable_rx_interrupt(void);	// on by default after init
void serial_disable_rx_interrupt(void);	// if disabling receiving is ever needed


#ifdef _HAS_UART1_
void serial2_init(void); 		   //defaults to 9600 bauds, 1 stop, no parity.
void serial2_init_9600b8N1(void); // 9600 bauds, 8 bits, 1 stop, no parity
void serial2_init_9600b7E1(void); // 9600 bauds, 7 bits, 1 stop, no parity
void serial2_putc(unsigned char ch);
void serial2_puts(char* string);
unsigned char serial2_getc(void); // blocking
unsigned char serial2_getc_nowait(void); // non-blocking, will return '\0' in no character present
#endif

#endif
