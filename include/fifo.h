/********************************************
 * Implementation of a circular fifo buffer
 * In order to use, initialize like this:
 *
 * 	#include "fifo.h"
 * 	#define BUF_SIZE 10
 * 	uint8_t buffer[BUF_SIZE];
 * 	fifo_t fifo;
 * 	...
 * 	    fifo_init (&fifo, buffer, BUF_SIZE);
 * 	...
 *
 * 	***************************************/

#ifndef FIFO_H
#define FIFO_H

#include <avr/io.h>
#include <avr/interrupt.h>

// metadata structure, includes everything but the buffer, which is implied with the pointers
typedef struct
{
	uint8_t volatile count;       // # of stored bytes
	uint8_t size;                 // max buffer size
	uint8_t *pread;               // read pointer
	uint8_t *pwrite;              // write pointer
	uint8_t read2end, write2end;  // # of bytes until overrun of read or write
} fifo_t;

// public interface
void fifo_init (fifo_t*, uint8_t* buf, const uint8_t size);
uint8_t fifo_put (fifo_t*, const uint8_t data);
uint8_t fifo_get_wait (fifo_t*);
int fifo_get_nowait (fifo_t*);	// this one is annoying, returns -1 if no data
int fifo_available(fifo_t *f);	// I made this one up instead

// private interface, not accessible from outsite (static members)

// add a character to the buffer
static inline uint8_t
_inline_fifo_put (fifo_t *f, const uint8_t data)
{
	// return if no more space
	if (f->count >= f->size)
		return 0;
	// get the write pointer
	uint8_t * pwrite = f->pwrite;
	// write the byte, THEN move pointer to the next position
	*(pwrite++) = data;
	// get the numbers of bytes left till end of buffer
	uint8_t write2end = f->write2end;
	// decrement numbers of bytes left, THEN test for none left
	if (--write2end == 0)
	{
		// if end of linear buffer, rewind at beginning of buffer (this is implemented as a circular buffer)
		write2end = f->size;	// size to end of linear buffer
		pwrite -= write2end;	// rewind the write pointer to the beginning
	}
	
	// store updated values of write pointers and write to end # of bytes
	f->write2end = write2end;
	f->pwrite = pwrite;

	// block interrupts while we update count, so it does not change in interrupt context while we read it
	uint8_t sreg = SREG;	// save status register
	cli();					// no interrupts
	f->count++;				// update count, we stored one more byte
	SREG = sreg;			// reset status register
							// ? no sei() code to re-enable interrupts. Or is the compiler putting one?
	return 1;
}

// read a character from the buffer
static inline uint8_t 
_inline_fifo_get (fifo_t *f)
{
	// get the read pointer
	uint8_t *pread = f->pread;
	// read the data, THEN advance the pointer
	uint8_t data = *(pread++);
	// get the size till end number
	uint8_t read2end = f->read2end;
	
	// decrement size, THEN test no space left
	if (--read2end == 0)
	{
		// if end of linear buffer, rewind to beginning (circular buffer)
		read2end = f->size;	// full buffer available for read
		pread -= read2end;	// move read pointer to beginning
	}
	
	// update the read pointer and read2end values
	f->pread = pread;
	f->read2end = read2end;
	
	// update count, making sure that nobody elses accesses it in interrupt
	uint8_t sreg = SREG;
	cli();
	f->count--;
	SREG = sreg;
	
	return data;
}

#endif /* FIFO_H */
