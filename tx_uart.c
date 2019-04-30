/*
 * Routines to bitbang transmit RS232 on a GPIO.
 *
 * Based on
 * http://nerdralph.blogspot.com/2014/01/avr-half-duplex-software-uart.html
 *
 * Copyright 2019 Jonathan McDowell <noodles@earth.li>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "tx_uart.h"

#ifdef F_CPU
#define TXDELAY (int)(((F_CPU / BAUD_RATE) - 7 + 1.5) / 3)
#else
#error Must define F_CPU
#endif

void uart_tx(char c)
{
	uint8_t oldSREG = SREG;
	PORTB |=  (1 << UART_TX);

	cli();
	asm volatile(
		"  cbi %[port], %[pin] \n"
		"  in r0, %[port]      \n"
		"  ldi r24, 3          \n"
		"  ldi r22, %[delay]   \n"
		"TxLoop:               \n"
		"  mov r23, r22        \n"
		"TxDelay:              \n"
		"  dec r23             \n"
		"  brne TxDelay        \n"
		"  bst %[ch], 0        \n"
		"  bld r0, %[pin]      \n"
		"  lsr r24             \n"
		"  ror %[ch]           \n"
		"  out %[port], r0     \n"
		"  brne TxLoop         \n"
		:
		: [port] "I" (_SFR_IO_ADDR(PORTB)),
		  [pin] "I" (UART_TX),
		  [delay] "I" (TXDELAY),
		  [ch] "r" (c)
		: "r0", "r22", "r23", "r24");

	SREG = oldSREG;
}

void uart_puts(char *s)
{
	while (*s)
		uart_tx(*(s++));
}

void uart_init(void)
{
	/* Set TX pin to output, set it high */
	PORTB |= (1 << UART_TX);
	DDRB |= (1 << UART_TX);
}
