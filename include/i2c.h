/************************************************
 * i2c.h
 *
 *  Created on: Aug 17, 2013
 *  Author: Marc Verdiell
 *  i2c library for AVR
 *  version 1.1
 *
 *	Implements a clean I2C Master with timeout to prevent bus lockup
 * 	Needs realtime.c for the timeout functionality
 *
 */

/*************************
 *  version 1.1
 *  Added support for dynamic timers with new realtime.c library
 *  Added commenting and examples on the main 3 functions
 ***********************/

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h> 		// uint8_t and companions
#include "toolbox.h"		// for typedef uint8_t bool;
#include <util/twi.h>		// Includes TW_READ and TW_WRITE

// Uncomment for debug, makes the i2c talkative on UART0. Comment out for silent operation
// saves a lot of DRAM too.

//#define I2C_DEBUG

// I2C clock in Hz. Set at 100 kHz.
// (F_CPU/SCL_CLOCK)-16)/2 must be >10
#define SCL_CLOCK  100000L

// I2C timeout. If the I2C bus hangs, this will abandon the transaction after the timeout
// Requires realtime.c library
#define I2C_TIMEOUT_ENABLED
#define I2C_TIMEOUT 50        // times out after 0.5s

// more logical with our naming conventions, to use in start() functions
#define I2C_READ  TW_READ
#define I2C_WRITE TW_WRITE

/****************************************************************
 * Main user functions
 * These 3 should be the only ones used for implementing a master
 ****************************************************************/

// init the i2c master. Set enablePullup to TRUE to enable 10k pullups (the usual case)
// for large systems you might need external 4.7k pullups.
void    i2c_init(bool enablePullup);

// i2c send data. You should usually put sendStop to True.
// Optionally, put SendStop to False only if you are going to do a read just afterwards
// (requesting data for example). This will generate a restart and save time in the I2C
// read transaction by skipping addressing.
// This function returns TRUE is an error occurred, FALSE if no error
// i2caddress is the 7 bit address (can be even or odd)
// To send a C string, write some code like this:
// 		char teststring[]="1234";
//		uint8_t i2clength=strlen(teststring);
// 		i2c_send_data(i2caddress,(uint8_t*)teststring, i2clength, TRUE);
bool i2c_send_data(uint8_t address, uint8_t *databuffer, uint8_t datalength, bool sendStop);

// i2c receive data. Might be preceded by a write beforehand which will tell the slave what type of data
// is requested, as there is no way to give any info in the receive request itself.
// This function returns TRUE is an error occurred, FALSE if no error
// i2caddress is the 7 bit address (can be even or odd)
// To receive a C string, write some code like this:
// 		uint8_t responselength=5;   // ask for 5 chars back?
// 		char response[responselength];
// 		i2c_receive_data(i2caddress, (uint8_t*) response, responselength);
bool i2c_receive_data(uint8_t address, uint8_t *databuffer, uint8_t datalength);

/***************************************************
 * Low level i2c master functions
 * You should not need to call these directly
 ***************************************************/
void    i2c_close();									// call if you ever want to release the pins for some other use
bool    i2c_start(uint8_t address, uint8_t readwrite);	// generates start condition, try to get hold of the bus
bool    i2c_stop(void);									// stop condition, release the bus, returns TRUE if it times out
bool    i2c_write(uint8_t databyte);					// write a byte
bool 	i2c_read(uint8_t *databyte, bool nack_last_byte);		// read a byte from the slave, set nack_last_byte to true for the last byte
uint8_t i2c_status();									// i2c status and error code from last operation


/**************************************************
 * SDL and SDA pin locations for internal pull ups
 *************************************************/

//------------------------------------------------------------------------------
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
// 168 and 328
// SCL is PC5
// SDA is PC4

#define I2C_SDA_PIN  4
#define I2C_SCL_PIN  5
#define I2C_PORT	 PORTC
//------------------------------------------------------------------------------
#elif defined(__AVR_ATmega128__)
// 128
// SCL is PD0
// SDA is PD1

#define I2C_SDA_PIN  1
#define I2C_SCL_PIN  0
#define I2C_PORT	 PORTD
//------------------------------------------------------------------------------
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Mega 1280 and 2560
// SCL is PD0
// SDA is PD1

#define I2C_SDA_PIN  1
#define I2C_SCL_PIN  0
#define I2C_PORT	 PORTD
//------------------------------------------------------------------------------


#else  // AVR CPU
#warning unknown CPU, i2c pull-ups not supported
#endif  // AVR CPU


#endif /* I2C_H_ */
