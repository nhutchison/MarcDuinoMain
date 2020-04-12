/*****************************************************
 * i2c.c
 *
 *  Created on: Aug 17, 2013
 *  Author: Marc Verdiell
 *  i2C master implemented using twi, polling
 *
 *****************************************************/

#include "i2c.h"
#include <avr/io.h> 	// This will include <avr/iom128.h> or , <avr/iom328p.h> register definitions
#include <util/twi.h>	// TW_... error, status codes. Includes TW_READ and TW_WRITE
#include <avr/interrupt.h> // for slave interrupt

// if using debug, include serial library
#ifdef I2C_DEBUG
#include "RS232.h"
#endif

// if using timeout (recommended), include the real-time library
#ifdef I2C_TIMEOUT_ENABLED
#include "realtime.h"
#endif

/* define CPU frequency in Mhz if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Private variable to keep status byte
static uint8_t i2c_status_;
#ifdef I2C_TIMEOUT_ENABLED
	rt_timer rt_timeout_i2c; // our timeout timer
#endif


/*****************
 * Private convenience function, for repetitive use
 * Performs a TWI operation, waits for completion and updates status
 * If timeout control is enabled, will return error (TRUE) on timeout
 *****************/
static bool i2c_execCmd(uint8_t cmdReg) {
	// send command
	TWCR = cmdReg;

#ifndef I2C_TIMEOUT_ENABLED
	// wait for operation to complete (no timeout)
	while (!(TWCR & (1 << TWINT)))
	// status bits.
	i2c_status_ = TWSR & 0xF8;
	return FALSE;
#else
	// wait for command to complete with timeout
	rt_timeout_i2c=I2C_TIMEOUT; 	// initialize timer
	while (!(TWCR & (1 << TWINT)))
	{
	  if(!rt_timeout_i2c)	// bus is hanged, we timed out
	  {
			#ifdef I2C_DEBUG
			uart0_puts("**i2c bus timed out \n\r");
			#endif
			// status bits.
			i2c_status_ = TWSR & 0xF8;
			// return with error
			return TRUE;
	  }
	}
	// return with no error
	i2c_status_ = TWSR & 0xF8;
	return FALSE;
#endif
}

/*****************
 * Returns status code of last i2c operation
 * **************/
uint8_t i2c_status()
{
	return i2c_status_;
}


/******************
 * Init
 * Sets bit rate at 100 kHz
 * Enable internal 10k pullups are 10k for small systems
 * For large systems, you might need to use external 4.7k
 * ********************/
void i2c_init(bool enablePullup)
{

	TWSR = 0;                         // no prescaler
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  // must be > 10 for stable operation

	// internal pull-up resistor support if we happen to know the correct pins
#ifdef I2C_SDA_PIN
	if (enablePullup)
	{
		set_port_pin(I2C_PORT,I2C_SDA_PIN);
		set_port_pin(I2C_PORT,I2C_SCL_PIN);
	}
#endif

	// Load data register with default content; release SDA
	TWDR = 0xff;

	// Enable TWI peripheral with interrupt disabled (set TWEN bit on)
	TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

#ifdef I2C_TIMEOUT_ENABLED
	rt_add_timer(&rt_timeout_i2c);
#endif
}

/******************
 * Close
 * If the i2c pins ever need to be released for another use
 * use this function to turn off the i2c transmitter
 * ********************/
void i2c_close()
{
	// Disable i2c
	TWCR = 0x00;
}

/******************
 * Start
 * Wait to get hold of the bus and generate a start condition
 * Address is 7 bit, will get shifted one bit right
 * Indicate if this is for a read or write by using TW_READ or TW_WRITE
 * Returns TRUE if error (i.e. the bus was busy)
 * ********************/
bool i2c_start(uint8_t address, uint8_t readwrite)
{
	// send START condition
	if(i2c_execCmd((1<<TWINT)|(1<<TWSTA)|(1<<TWEN)))
	{
		#ifdef I2C_DEBUG
		uart0_puts("**i2c start condition timed-out \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}

	// return error if the bus is busy
	if (i2c_status() != TW_START && i2c_status() != TW_REP_START)
	{
		#ifdef I2C_DEBUG
		uart0_puts("**i2c start condition could not be transmitted \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}

	// send device address and direction byte
	//TWDR = (address<<1 | (readwrite & 0x01));
	TWDR = (address<<1 | readwrite);
	if(i2c_execCmd((1<<TWINT) | (1<<TWEN)))
	{
		#ifdef I2C_DEBUG
		uart0_puts("**i2c address send timed-out \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}

	// make sure to release the bus if there is an error other than bus timeout
	if (readwrite == TW_READ)
	{
		if(i2c_status() != TW_MR_SLA_ACK)
		{
			#ifdef I2C_DEBUG
			uart0_puts("**i2c start READ ACK failed \n\r");
			#endif
			i2c_stop();
			return TRUE;
		}
	}
	else
	{
		if(i2c_status() != TW_MT_SLA_ACK)
		{
			#ifdef I2C_DEBUG
			uart0_puts("**i2c start TRANSMIT ACK failed \n\r");
			#endif
			i2c_stop();
			return TRUE;
		}
	}
	return FALSE;
}

/**************************
 * Stop
 * Release the bus
 * ***********************/
bool i2c_stop()
{
	// send stop, no time out
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

#ifndef I2C_TIMEOUT_ENABLED
	// wait until stop condition is executed and bus released, no time out
	while (TWCR & (1 << TWSTO));
	i2c_status_ = TWSR & 0xF8;
	return FALSE;
#else
	// wait for stop to complete with timeout
	rt_timeout_i2c=I2C_TIMEOUT; 	// initialize timer
	while (TWCR & (1 << TWSTO))
	{
	  if(!rt_timeout_i2c)	// bus is hanged, we timed out
	  {
			#ifdef I2C_DEBUG
			uart0_puts("**i2c bus timed out in stop \n\r");
			#endif
			// return with error
			i2c_status_ = TWSR & 0xF8;
			return TRUE;
	  }
	}
	// return with no error
	i2c_status_ = TWSR & 0xF8;
	return FALSE;
#endif
}

/**************************
 * Write
 * Writes a byte on the bus
 * Returns FALSE is the slave ACKed the byte, TRUE (error) if it is NACKed
 * ***********************/
bool i2c_write(uint8_t databyte)
{

	// write the byte on the bus
	TWDR = databyte;
	if(i2c_execCmd((1 << TWINT) | (1 << TWEN)))
	{
		#ifdef I2C_DEBUG
		uart0_puts("\n\r i2c write timed-out \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}
	//in case of NACK, issue a stop, so user doesn't have to remember
	if(i2c_status() != TW_MT_DATA_ACK)
	{
		#ifdef I2C_DEBUG
		uart0_puts("\n\r i2c write NACKed by slave \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}
	else return FALSE;
}

/**************************
 * Read
 * Read a single byte requested from the slave
 * if nack_last_byte is false, another byte is requested by sending an ACK
 * Set nack_last_byte to true when this is the last byte to read
 * so no more bytes are requested by signaling a NACK to the slave
 * ***********************/
bool i2c_read(uint8_t *databyte, bool nack_last_byte)
{
	if(i2c_execCmd((1 << TWINT) | (1<<TWEN) | (nack_last_byte ? 0 : (1<<TWEA))))
	{
		#ifdef I2C_DEBUG
		uart0_puts("\n\r i2c read timed-out \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}

	*databyte=TWDR;
	return FALSE;
}

/***************************
 * Send Data Block
 * That's the only send function you should use in a master
 * Generates start condition, sends a block of data,
 * and does stop or prepares for read/restart if stop is set to FALSE
 * Returns TRUE if an error happened (byte not ACK'ed)
 * Returns FALSE if no error
 ****************************/

bool i2c_send_data(uint8_t address, uint8_t *databuffer, uint8_t datalength, bool sendStop)
{
	// check that we got a pointer and at least one character
	if(databuffer==0 || datalength==0)
	{
			// we could be in a restart, make sure we issue a stop
		#ifdef I2C_DEBUG
		uart0_puts("\n\r i2c_send_data called with null data or length \n\r");
		#endif
		i2c_stop();
		return TRUE;
	}

	// generate start condition, return if error
	if(i2c_start(address, I2C_WRITE)) return TRUE;

	// send the bytes, return if error
	for(uint8_t i=0; i<datalength; i++)
	{
		if(i2c_write(databuffer[i])) return TRUE;
	}

	// only send a stop if requested, if not, user needs to restart and stop with a read
	if(sendStop) i2c_stop();
	return FALSE;
}

/***************************
 * Receive Data Block
 * That's the only read function you should use in a master
 * Generates start or restart condition, reads a block of data,
 * and sends a stop at the end
 * Returns TRUE if an error happened
 * Returns FALSE if no error
 ****************************/
bool i2c_receive_data(uint8_t address, uint8_t *databuffer, uint8_t datalength)
{
	uint8_t databyte;
	// check that we got a pointer and at least one character
	if(databuffer==0 || datalength==0)
	{
		#ifdef I2C_DEBUG
		uart0_puts("\n\r i2c_receive_data called with null data or length \n\r");
		#endif
		// we could be in a restart, make sure we issue a stop
		i2c_stop();
		return TRUE;
	}

	// generate start or re-start condition
	if(i2c_start(address, I2C_READ))
	{
			// an error happened. We could be in a restart, make sure we issue a stop
			i2c_stop();
			return TRUE;
	}

	// read all bytes except the last one
	uint8_t i=0;
	while(i<datalength-1)
	{
		// read with ACK for requesting more bytes
		if(i2c_read(&databyte, FALSE))
		{
			//error happened, bus did hang while reading.
			i2c_stop();
			return TRUE;
		}
		// only fill the databuffer with valid bytes
		databuffer[i]=databyte;
		i++;
	}

	// this should now be the last read (datalength-1), NACK it
	if(i2c_read(&databyte, TRUE))
	{
		//error happened, bus did hang while reading.
		i2c_stop();
		return TRUE;
	}
	databuffer[i]=databyte;

	// generate stop condition
	i2c_stop();
	return FALSE;
}

