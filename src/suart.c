/*
 * softuart.c
 * Software serial communication implementation
 * Does not use timers, just delays
 * For best timing compile with interrupts off (define SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT),
 * but at 9600 bauds with short interrupts it is not necessary
 * v1.0 - clean C code, inspired from complex arduino C++ library
 * v1.1 - correct assembly routine so it compiles in gcc 4.4
 * v1.2 - add support for 2 suart pins
 * v1.3 - switch for MarcDuino v1 and v2 Suart2 pin in header
 *
*/

/***** DEBUG of suart2
#include <stdlib.h>			// for itoa(), integer to string conversion
#include <stdio.h>			// for sprintf(), don't use if you are short on memory
*********/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "toolbox.h"
#include "suart.h"

// Delay lookup table

typedef struct _DELAY_TABLE
{
  long baud;
  unsigned short rx_delay_centering;
  unsigned short rx_delay_intrabit;
  unsigned short rx_delay_stopbit;
  unsigned short tx_delay;
} DELAY_TABLE;

// delay tables are available for 8 MHz, 16 MHz and 20 MHz clocks only
// stored in program memory to save RAM

#if F_CPU == 16000000

static const DELAY_TABLE PROGMEM suart_delay_table[] =
{
  //  baud    rxcenter   rxintra    rxstop    tx
  { 115200,   1,         17,        17,       12,    },
  { 57600,    10,        37,        37,       33,    },
  { 38400,    25,        57,        57,       54,    },
  { 31250,    31,        70,        70,       68,    },
  { 28800,    34,        77,        77,       74,    },
  { 19200,    54,        117,       117,      114,   },
  { 14400,    74,        156,       156,      153,   },
  { 9600,     114,       236,       236,      233,   },
  { 4800,     233,       474,       474,      471,   },
  { 2400,     471,       950,       950,      947,   },
  { 1200,     947,       1902,      1902,     1899,  },
  { 300,      3804,      7617,      7617,     7614,  },
};

const int XMIT_START_ADJUSTMENT = 5;

#elif F_CPU == 8000000

static const DELAY_TABLE suart_delay_table[] PROGMEM =
{
  //  baud    rxcenter    rxintra    rxstop  tx
  { 115200,   1,          5,         5,      3,      },
  { 57600,    1,          15,        15,     13,     },
  { 38400,    2,          25,        26,     23,     },
  { 31250,    7,          32,        33,     29,     },
  { 28800,    11,         35,        35,     32,     },
  { 19200,    20,         55,        55,     52,     },
  { 14400,    30,         75,        75,     72,     },
  { 9600,     50,         114,       114,    112,    },
  { 4800,     110,        233,       233,    230,    },
  { 2400,     229,        472,       472,    469,    },
  { 1200,     467,        948,       948,    945,    },
  { 300,      1895,       3805,      3805,   3802,   },
};

const int XMIT_START_ADJUSTMENT = 4;

#elif F_CPU == 20000000

// 20MHz support courtesy of the good people at macegr.com.
// Thanks, Garrett!

static const DELAY_TABLE PROGMEM suart_delay_table[] =
{
  //  baud    rxcenter    rxintra    rxstop  tx
  { 115200,   3,          21,        21,     18,     },
  { 57600,    20,         43,        43,     41,     },
  { 38400,    37,         73,        73,     70,     },
  { 31250,    45,         89,        89,     88,     },
  { 28800,    46,         98,        98,     95,     },
  { 19200,    71,         148,       148,    145,    },
  { 14400,    96,         197,       197,    194,    },
  { 9600,     146,        297,       297,    294,    },
  { 4800,     296,        595,       595,    592,    },
  { 2400,     592,        1189,      1189,   1186,   },
  { 1200,     1187,       2379,      2379,   2376,   },
  { 300,      4759,       9523,      9523,   9520,   },
};

const int XMIT_START_ADJUSTMENT = 6;

#else

#error This version of SoftUart supports only 20, 16 and 8MHz processors

#endif


// variables used by this module [NEED TO RENAME]

static uint16_t _rx_delay_centering;
static uint16_t _rx_delay_intrabit;
static uint16_t _rx_delay_stopbit;
static uint16_t _tx_delay;

// tuned delay loop
inline void suart_tunedDelay(uint16_t delay) {
  uint8_t tmp=0;

  asm volatile("sbiw    %0, 0x01 \n\t"
    "ldi %1, 0xFF \n\t"
    "cpi %A0, 0xFF \n\t"
    "cpc %B0, %1 \n\t"
    "brne .-10 \n\t"
//    : "+r" (delay), "+a" (tmp)
//	for gcc 4.4 compiler, need to use 'w' qualifier to force it to use upper registers for sbiw
//	instruction. Any register 'r' will not work with sbiw causing asm error.
    : "+w" (delay), "+a" (tmp)
    : "0" (delay)
    );
}

inline void suart_tx_pin_write(uint8_t pin_state)
{
    digitalWrite(SUART_TX_PORT, SUART_TX_PIN, pin_state);
}

// Module Init
void suart_init(long speed)
{
  _rx_delay_centering=0;
  _rx_delay_intrabit=0;
  _rx_delay_stopbit=0;
  _tx_delay=0;

  // set Tx pin for output
  digitalMode(SUART_TX_PORT, SUART_TX_PIN, OUTPUT);
  digitalWrite(SUART_TX_PORT, SUART_TX_PIN, HIGH);

  // read delay settings from table to match baud rate

  uint8_t i;
  for (i=0; i<sizeof(suart_delay_table)/sizeof(suart_delay_table[0]); ++i)
  {
    long baud = pgm_read_dword(&suart_delay_table[i].baud);
    if (baud == speed)
    {
      _rx_delay_centering = pgm_read_word(&suart_delay_table[i].rx_delay_centering);
      _rx_delay_intrabit = pgm_read_word(&suart_delay_table[i].rx_delay_intrabit);
      _rx_delay_stopbit = pgm_read_word(&suart_delay_table[i].rx_delay_stopbit);
      _tx_delay = pgm_read_word(&suart_delay_table[i].tx_delay);
      break;
    }
  }
}


void suart_putc(uint8_t b)
{
	uint8_t mask;

  if (_tx_delay == 0)
    return;

#ifdef SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT
  uint8_t oldSREG = SREG;
  cli();  // turn off interrupts for a clean txmit
#endif

  // Write the start bit
  suart_tx_pin_write(LOW);
  suart_tunedDelay(_tx_delay + XMIT_START_ADJUSTMENT);

  // Write each of the 8 bits

	for (mask = 0x01; mask; mask <<= 1)
	{
	  if (b & mask) // choose bit
		suart_tx_pin_write(HIGH); // send 1
	  else
		suart_tx_pin_write(LOW); // send 0

	  suart_tunedDelay(_tx_delay);
	}

	suart_tx_pin_write(HIGH); // restore pin to natural state

#ifdef SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT
  SREG = oldSREG; // turn interrupts back on
  sei();
#endif

  suart_tunedDelay(_tx_delay);
}

void suart_puts(char* string)
{
	uint8_t i=0;
	while( (string[i]!='\0') & (i<255))
	{
		suart_putc(string[i]);
	 	i++;
	}
}

/*************************************************************************
suart_puts_p()
Transmit string stored in program memory (so it does not take RAM space)
String has to be declared as
#include <avr/pgmspace.h> // optional, already included in this header
const char string[] PROGMEM = "Hello World";
suart_puts_p(string);
**************************************************************************/
void suart_puts_p(const char *progmem_s )
{
    register char c;
    while ( (c = pgm_read_byte(progmem_s++)) )
      suart_putc(c);
}

// **** suart2 functions for dual port ********
#ifdef SUART_DUAL_PORT

static uint16_t _rx2_delay_centering;
static uint16_t _rx2_delay_intrabit;
static uint16_t _rx2_delay_stopbit;
static uint16_t _tx2_delay;

inline void suart2_tx_pin_write(uint8_t pin_state)
{
    digitalWrite(SUART2_TX_PORT, SUART2_TX_PIN, pin_state);
}

// Module Init
void suart2_init(long speed)
{
  _rx2_delay_centering=0;
  _rx2_delay_intrabit=0;
  _rx2_delay_stopbit=0;
  _tx2_delay=0;

  // set Tx pin for output
  digitalMode(SUART2_TX_PORT, SUART2_TX_PIN, OUTPUT);
  digitalWrite(SUART2_TX_PORT, SUART2_TX_PIN, HIGH);

  // read delay settings from table to match baud rate

  uint8_t i;
  for (i=0; i<sizeof(suart_delay_table)/sizeof(suart_delay_table[0]); ++i)
  {
    long baud = pgm_read_dword(&suart_delay_table[i].baud);
    if (baud == speed)
    {
      _rx2_delay_centering = pgm_read_word(&suart_delay_table[i].rx_delay_centering);
      _rx2_delay_intrabit = pgm_read_word(&suart_delay_table[i].rx_delay_intrabit);
      _rx2_delay_stopbit = pgm_read_word(&suart_delay_table[i].rx_delay_stopbit);
      _tx2_delay = pgm_read_word(&suart_delay_table[i].tx_delay);
      break;
    }
  }

  /**** debug
  suart2_puts("\r\nsuart2 output test\r\n");
  char string[64];
  sprintf(string, "PORT: %2d PIN: %2d \r\n", SUART2_TX_PORT, SUART2_TX_PIN);
  suart2_puts(string);
  ***********/
}


void suart2_putc(uint8_t b)
{
	uint8_t mask;

  if (_tx2_delay == 0)
    return;

#ifdef SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT
  uint8_t oldSREG = SREG;
  cli();  // turn off interrupts for a clean txmit
#endif

  // Write the start bit
  suart2_tx_pin_write(LOW);
  suart_tunedDelay(_tx2_delay + XMIT_START_ADJUSTMENT);

  // Write each of the 8 bits

	for (mask = 0x01; mask; mask <<= 1)
	{
	  if (b & mask) // choose bit
		suart2_tx_pin_write(HIGH); // send 1
	  else
		suart2_tx_pin_write(LOW); // send 0

	  suart_tunedDelay(_tx2_delay);
	}

	suart2_tx_pin_write(HIGH); // restore pin to natural state

#ifdef SUART_TURN_OFF_INTERRUPTS_WHILE_TRANSMIT
  SREG = oldSREG; // turn interrupts back on
  sei();
#endif

  suart_tunedDelay(_tx2_delay);
}

void suart2_puts(char* string)
{
	uint8_t i=0;
	while( (string[i]!='\0') & (i<255))
	{
		suart2_putc(string[i]);
	 	i++;
	}
}

void suart2_puts_p(const char *progmem_s )
{
    register char c;
    while ( (c = pgm_read_byte(progmem_s++)) )
      suart2_putc(c);
}




#endif
