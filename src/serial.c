/**************************************
 *  serial.c
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
 *  - switch to interrupt driven, fully buffered
 *  - second UART still polled, not updated yet
 *  v2.1 06.01/2015
 *  - made serial_puts wait if output buffer is full
 *  - created serial_puts_nowait if no waiting is required (faster too for fast serial speeds)
 *
 *************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "binary.h"
#include "fifo.h"

// Fifo buffers for input and output

uint8_t inbuf[BUFSIZE_IN];
fifo_t infifo;

uint8_t outbuf[BUFSIZE_OUT];
fifo_t outfifo;


void serial_init(uint16_t baudrate)
{
  /************ explanation of registers **************
	// USART1 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	// USART1 Baud rate: 9600
	// UBRR=(CPUCLOCK/(16*BAUDRATE))-1

	// UCSR1A bit0=MPMC  (set for addressed serial, normally 0)
	// UCSR1A bit1=U2X1  (set to Double the USART Transmission Speed, normally 0)
	// UCSR1A bit2=UPE1  (Parity error flag)
	// UCSR1A bit3=DOR1  (Data Overrun error flag)
	// UCSR1A bit4=FE1 	 (Frame error flag)
	// UCSR1A bit5=UDRE1 (USART Data Register Empty, poll or interrupt to write)
	// UCSR1A bit6=TXC1  (transmit complete - only for half duplex)
	// UCSR1A bit7=RXC1  (receive complete - poll or generate interrupt to read)

	// UCSR1B bit0=TXB8n  (Transmit Data Bit 8 for 9 bit frame schemes)
	// UCSR1B bit1=RXB8n  (Receive Data Bit 8 for 9 bit frame schemes)
	// UCSR1B bit2=UCSZn2 (Number of data bits, set in combo with UCSZn1:0 bits in UCSRnC for 9 bit scheme)
	// UCSR1B bit3=TXEN1  (Transmitter Enable, set to 1 during init)
	// UCSR1B bit4=RXEN1  (Receiver Enable, set to 1 during init)
	// UCSR1B bit5=UDRIE1 (USART Data Register Empty Interrupt Enable, 0 for polling scheme)
	// UCSR1B bit6=TXCIE1 (TX Complete Interrupt Enable, 0 for polling scheme)
	// UCSR1B bit7=RXCIE1 (RX Complete Interrupt Enable, half duplex schemes, 0 for polling scheme)

	// UCSR1C bit0=UCPOLn 	(Clock Polarity, for synchronous mode only, write to 0 for asynchronous)
	// UCSR1C bit1=UCSZn0 	(Number of data bits, 1 8bits, 0 7bits, assuming UCSZn1=0)
	// UCSR1C bit2=UCSZn1 	(Number of data bits, 1      , 1      , assuming UCSZn1=1)                 )
	// UCSR1C bit3=USBSn    (Stop Bit Select, 0= 1 stop bit, 1= 2 stop bits, for tx only, rx just ignores it)
	// UCSR1C bit4=UPMn0	(Parity Mode bit 0: 0 disabled, 1 reserved, 0 even, 1 odd)
	// UCSR1C bit5=UPMn1 	(Parity Mode bit 1: 0         , 0         , 1     , 1    )
	// UCSR1C bit6=UMSELn 	(USART Mode Select, normally Asynchronous 0, or 1 for synchronous)
	// UCSR1C bit7=URSELn 	Reserved (always write to 0)

	// Enable receiver and transmitter
	UCSR1A=0x00; // no interrupt scheme
	UCSR1B = (1<<RXEN)|(1<<TXEN); // enable transmit and receive
	// Set frame format: 8data, 2stop bit
	UCSR1C = (1<<USBS)|(3<<UCSZ0);
	// Set frame format: 8 data, 1 stop:
	//UCSR1C = (3<<UCSZ0);
	// Set Parity Check: UPM1 to 1
	// Set Parity Type: UPM0 to odd or even

   *********************************************/
    uint8_t dummydata;		// to avoid warning from compiler later
    uint8_t sreg = SREG;	// save status register

    // bit rate calculation
	uint16_t ubrr = (uint16_t) ((uint32_t) F_CPU/(16UL*baudrate) - 1);
	UBRR0H=(uint8_t) (ubrr>>8);	//UBRRH = (uint8_t) (ubrr>>8);
	UBRR0L=(uint8_t) (ubrr);	//UBRRL = (uint8_t) (ubrr);

	// Disable interrupts for a short while
	cli();

	// all defaults for UCSRA
	UCSR0A=0x00;
	// turn on Rx, Tx and set to generate interrupts when Rx got a character
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	// Data mode 8N1,  asynchronous (UCSZ00=8bit,
	UCSR0C=(1 << UCSZ01) | (1 << UCSZ00);

	// Flush Receive-Buffer
	do
	{
		dummydata=UDR0; // put the read in something to avoid compiler warning.
	}
	while (UCSR0A & (1 << RXC0)); // polling the receive complete bit

   // Reset Receive and Transmit Complete Flags
	UCSR0A = (1 << RXC0) | (1 << TXC0);

    // Re-enable interrupts (don't see sei() call?)
    // Oh I see not needed, the global interrupt bit was part of saved SREG!)
    SREG = sreg;
    // adding it back, or forgetting might not allow serial to work
    sei();
    // Initialize input and output FIFOs
    fifo_init (&infifo,   inbuf, BUFSIZE_IN);
    fifo_init (&outfifo, outbuf, BUFSIZE_OUT);
}

void serial_init_9600b8N1(void) // 9600 bauds, 8 bits, 1 stop, no parity
{
	serial_init(9600);
}

void serial_init_9600b7E1(void) // 9600 bauds, 7 bits, 1 stop, even parity
{
	serial_init(9600);
	// override control register 0C
	UCSR0C=B00100100;		// 7 bits, 1 stop bit, even parity, asynchronous
}

/***** Receive Interrupt Enable
 *
 * enables automatic call to ISR(UART0_RECEIVE_INTERRUPT) when a character is received
 * character must be read during interrupt OR interrupt must be explicitly disabled by interrupt routine
 * to avoid a repeat interrupt call after the routine is complete.
 * So your code should look something like:
 * 	ISR(USART_RX_vect)
 * 	{
 * 		charread=serial_getc(void)
 * 	}
 *
 ******/

void serial_enable_rx_interrupt(void)
{
    // enable receive complete interrupt
	// (note: this is enabled by default after init)
    UCSR0B |= _BV(RXCIE0);
}

void serial_disable_rx_interrupt(void)
{
	// disable receive complete interrupt
    UCSR0B &= ~_BV(RXCIE0);
}

// generic one not implemented yet
//void uart1_init(int baudrate, int databits, int stopbits, int parity){}

// Input Interrupt - can't do any simpler, just store input in FIFO...
ISR (USART_RX_vect)
{
    _inline_fifo_put (&infifo, UDR0);
}

// Output Interrupt
// Read a byte from the output fifo and send it
// Another interrupt will be triggered as soon as one byte is done sending.
// The interrupt routine will deactivate itself when the Fifo is empty
// The serial_putc routine resets the interrupt on to start the process
ISR(USART_UDRE_vect)
{
    // send out byte if there is one waiting
	if (outfifo.count > 0)
       UDR0 = _inline_fifo_get (&outfifo);
	// no more bytes, deactivate send interrupts
    else
        UCSR0B &= ~(1 << UDRIE0);
}

// Add character to output buffer, and try to send
// returns 0 if buffer full, 1 if success
uint8_t serial_putc(unsigned char ch)
{
	// add character to output buffer
	uint8_t ret = fifo_put (&outfifo, ch);
	// set interrupt on empty out queue to call ISR
	UCSR0B |= (1 << UDRIE0);
	return ret;
}

// check if receive characters available
uint8_t serial_available()
{
	return fifo_available(&infifo);
}

// check if ready to transmit (output buffer full)
// use this one to wait and avoid overrunning the output buffer
uint8_t serial_tx_complete()
{
	return !fifo_available(&outfifo);
}

// this one is only valid if some char available, if not returns 0xFF
// so check serial_available() beforehand
unsigned char serial_getc (void)
{
    return fifo_get_nowait (&infifo);
}

// returns -1 if no characters available, only works for 7 bit ASCII
int8_t serial_getc_nowait (void)
{
    return fifo_get_nowait (&infifo);
}

unsigned char serial_getc_wait (void)
{
    return fifo_get_wait (&infifo);
}

// If the output buffer is full, this will wait for serial buffer clear before returning
// all characters are sent guaranteed
void serial_puts(char* string)
{
	uint8_t i=0;
	while( (string[i]!='\0') & (i<255))
	{
		while(!serial_tx_complete()){};
		serial_putc(string[i]);
	 	i++;
	}
}

//this version will return no matter what, not waiting for serial buffer to clear
//if the output buffer is full some character will be lost
uint8_t serial_puts_nowait(char* string)
{
	uint8_t i=0;
	uint8_t noerror=1;
	while( (string[i]!='\0') & (i<255))
	{
		noerror &= serial_putc(string[i]); // if an overflow occurs, force noerror to 0
	 	i++;
	}
	return noerror;
}


/*************************************************************************
serial_puts_p()
Transmit string stored in program memory (so it does not take RAM space)
String has to be declared as
#include <avr/pgmspace.h> // optional, already included in this header
const char string[] PROGMEM = "Hello World";
serial_puts_p(string);
**************************************************************************/
void serial_puts_p(const char *progmem_s )
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) )
    {
    	while(!serial_tx_complete()){};	// wait for serial buffer availability
    	serial_putc(c);
    }
}


#ifdef _HAS_UART1_
//--------Init Uart interface --------------------
// this original Init is minimal in term of code - hard wired, with explanations on how to change bits
void uart1_init(void) {

	// USART1 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	// USART1 Baud rate: 9600
	UCSR1A=0x00;
	UCSR1B=0x18;     		//(00011000)
	UCSR1C=0x06;
	UBRR1H=0x00;
	UBRR1L=0x67;			//103 UBRR=(CPUCLOCK/(16*BAUDRATE))-1
}

void uart1_init_9600b8N1(void) // 9600 bauds, 8 bits, 1 stop, no parity
{
	UCSR1A=B00000000;		// default OK, normal clocking
	UCSR1B=B00011000;     	// enable Tx and Rx
	UCSR1C=B00000110;		// 8 bits, 1 stop bit, no parity asynchronous
	UBRR1H=0x00;
	UBRR1L=(unsigned char)(F_CPU/16/9600-1);	//9600 bauds, UBRR=(CPUCLOCK/(16*BAUDRATE))-1


	/***************** example code:
	// Baud calculation for Asynchronous Normal Mode (U2X=0)
	#define BAUD 9600
	#define MYUBRR FOSC/16/BAUD-1
	void USART_Init( unsigned int ubrr )
	{

	UBRR1H = (unsigned char)(ubrr>>8);
	UBRR1L = (unsigned char)ubrr;

	// UCSR1A bit0=MPMC  (set for addressed serial, normally 0)
	// UCSR1A bit1=U2X1  (set to Double the USART Transmission Speed, normally 0)
	// UCSR1A bit2=UPE1  (Parity error flag)
	// UCSR1A bit3=DOR1  (Data Overrun error flag)
	// UCSR1A bit4=FE1 	 (Frame error flag)
	// UCSR1A bit5=UDRE1 (USART Data Register Empty, poll or interrupt to write)
	// UCSR1A bit6=TXC1  (transmit complete - only for half duplex)
	// UCSR1A bit7=RXC1  (receive complete - poll or generate interrupt to read)

	// UCSR1B bit0=TXB8n  (Transmit Data Bit 8 for 9 bit frame schemes)
	// UCSR1B bit1=RXB8n  (Receive Data Bit 8 for 9 bit frame schemes)
	// UCSR1B bit2=UCSZn2 (Number of data bits, set in combo with UCSZn1:0 bits in UCSRnC for 9 bit scheme)
	// UCSR1B bit3=TXEN1  (Transmitter Enable, set to 1 during init)
	// UCSR1B bit4=RXEN1  (Receiver Enable, set to 1 during init)
	// UCSR1B bit5=UDRIE1 (USART Data Register Empty Interrupt Enable, 0 for polling scheme)
	// UCSR1B bit6=TXCIE1 (TX Complete Interrupt Enable, 0 for polling scheme)
	// UCSR1B bit7=RXCIE1 (RX Complete Interrupt Enable, half duplex schemes, 0 for polling scheme)

	// UCSR1C bit0=UCPOLn 	(Clock Polarity, for synchronous mode only, write to 0 for asynchronous)
	// UCSR1C bit1=UCSZn0 	(Number of data bits, 1 8bits, 0 7bits, assuming UCSZn2=0)
	// UCSR1C bit2=UCSZn1 	(Number of data bits, 1      , 1      ,                  )
	// UCSR1C bit3=USBSn    (Stop Bit Select, 0= 1 stop bit, 1= 2 stop bits, for tx only, rx just ignores it)
	// UCSR1C bit4=UPMn0	(Parity Mode bit 0: 0 disabled, 1 reserved, 0 even, 1 odd)
	// UCSR1C bit5=UPMn1 	(Parity Mode bit 1: 0         , 0         , 1     , 1    )
	// UCSR1C bit6=UMSELn 	(USART Mode Select, normally Asynchronous 0, or 1 for synchronous)
	// UCSR1C bit7=Reserved (always write to 0)

	// Enable receiver and transmitter
	UCSR1A=0x00; // no interrupt scheme
	UCSR1B = (1<<RXEN)|(1<<TXEN); // enable transmit and receive
	// Set frame format: 8data, 2stop bit
	UCSR1C = (1<<USBS)|(3<<UCSZ0);
	// Set frame format: 8 data, 1 stop:
	//UCSR1C = (3<<UCSZ0);
	// Set Parity Check: UPM1 to 1
	// Set Parity Type: UPM0 to odd or even

	}

	***********************************/
}

void uart1_init_9600b7E1(void) // 9600 bauds, 7 bits, 1 stop, no parity
{
	// USART1 initialization
	// Communication Parameters: 7 Data, 1 Stop, Even Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	// USART1 Baud rate: 9600

	UCSR1A=B00000000;		// default OK, normal clocking
	UCSR1B=B00011000;     	// enable Tx and Rx
	UCSR1C=B00100100;		// 7 bits, 1 stop bit, even parity, asynchronous
	UBRR1H=0x00;
	UBRR1L=(unsigned char)(F_CPU/16/9600-1);	//9600 bauds, UBRR=(CPUCLOCK/(16*BAUDRATE))-1


	/***************** example code:
	// Baud calculation for Asynchronous Normal Mode (U2X=0)
	#define BAUD 9600
	#define MYUBRR FOSC/16/BAUD-1
	void USART_Init( unsigned int ubrr )
	{

	UBRR1H = (unsigned char)(ubrr>>8);
	UBRR1L = (unsigned char)ubrr;

	// UCSR1A bit0=MPMC  (set for addressed serial, normally 0)
	// UCSR1A bit1=U2X1  (set to Double the USART Transmission Speed, normally 0)
	// UCSR1A bit2=UPE1  (Parity error flag)
	// UCSR1A bit3=DOR1  (Data Overrun error flag)
	// UCSR1A bit4=FE1 	 (Frame error flag)
	// UCSR1A bit5=UDRE1 (USART Data Register Empty, poll or interrupt to write)
	// UCSR1A bit6=TXC1  (transmit complete - only for half duplex)
	// UCSR1A bit7=RXC1  (receive complete - poll or generate interrupt to read)

	// UCSR1B bit0=TXB8n  (Transmit Data Bit 8 for 9 bit frame schemes)
	// UCSR1B bit1=RXB8n  (Receive Data Bit 8 for 9 bit frame schemes)
	// UCSR1B bit2=UCSZn2 (Number of data bits, set in combo with UCSZn1:0 bits in UCSRnC for 9 bit scheme)
	// UCSR1B bit3=TXEN1  (Transmitter Enable, set to 1 during init)
	// UCSR1B bit4=RXEN1  (Receiver Enable, set to 1 during init)
	// UCSR1B bit5=UDRIE1 (USART Data Register Empty Interrupt Enable, 0 for polling scheme)
	// UCSR1B bit6=TXCIE1 (TX Complete Interrupt Enable, 0 for polling scheme)
	// UCSR1B bit7=RXCIE1 (RX Complete Interrupt Enable, half duplex schemes, 0 for polling scheme)

	// UCSR1C bit0=UCPOLn 	(Clock Polarity, for synchronous mode only, write to 0 for asynchronous)
	// UCSR1C bit1=UCSZn0 	(Number of data bits, 1 8bits, 0 7bits, assuming UCSZn2=0)
	// UCSR1C bit2=UCSZn1 	(Number of data bits, 1      , 1      ,                  )
	// UCSR1C bit3=USBSn    (Stop Bit Select, 0= 1 stop bit, 1= 2 stop bits, for tx only, rx just ignores it)
	// UCSR1C bit4=UPMn0	(Parity Mode bit 0: 0 disabled, 1 reserved, 0 even, 1 odd)
	// UCSR1C bit5=UPMn1 	(Parity Mode bit 1: 0         , 0         , 1     , 1    )
	// UCSR1C bit6=UMSELn 	(USART Mode Select, normally Asynchronous 0, or 1 for synchronous)
	// UCSR1C bit7=Reserved (always write to 0)

	// Enable receiver and transmitter
	UCSR1A=0x00; // no interrupt scheme
	UCSR1B = (1<<RXEN)|(1<<TXEN); // enable transmit and receive
	// Set frame format: 8data, 2stop bit
	UCSR1C = (1<<USBS)|(3<<UCSZ0);
	// Set frame format: 8 data, 1 stop:
	//UCSR1C = (3<<UCSZ0);
	// Set Parity Check: UPM1 to 1
	// Set Parity Type: UPM0 to odd or even

	}

	***********************************/
}

// generic one not implemented yet
//void uart1_init(int baudrate, int databits, int stopbits, int parity){}

void uart1_putc(unsigned char ch)
{
	// This will only work for schemes with equal or less than 8 data bits
	// Polling method: wait for empty transmit buffer
	// by polling the Data Register Empty Flag (UDRE1)in the Control Register (UCSR1A)
	while(!(UCSR1A & (1<<UDRE1)));
	// send data by writing it to the send buffer
	UDR1 = ch;

}

void uart1_puts(char* string)
{
	uint8_t i=0;
	while( (string[i]!='\0') & (i<255))
	{
		uart1_putc(string[i]);
	 	i++;
	}
}

unsigned char uart1_getc(void)
{
	// This will only work for schemes with equal or less than 8 data bits
	// Polling method: wait for receive complete
	// by polling the Receive Complete Flag (RXC1) in the Control Register (UCSR1A)
	while(!(UCSR1A & (1<<RXC1)));

	// No error code here, complete code could check UCSR1A for
	// Frame Error (FE), Data OverRun (DOR) and Parity Error (UPE)

	// return received data from buffer
	return UDR1;
}


unsigned char uart1_getc_nowait(void)
{
	// this is non blocking, returns immediately with 0 if no characters present
	// wait for data to be received
	if((UCSR1A & (1<<RXC1)))
		return UDR1;	// get and return received data from buffer
	else
		return 0; 		// return immediately if no data
}
#endif
