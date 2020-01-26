/*
 * 433MHz receiver routines using Pin Change interrupt
 * Heavily based on rc-switch: https://github.com/sui77/rc-switch
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "r433.h"
#include "timer.h"
#include "tx_uart.h"

#define RECEIVER_PIN	0	/* Pin 0 on port B */
#define MAX_CHANGES	80

int nReceiveTolerance = 60;
const unsigned int nSeparationLimit = 3500;
unsigned int timings[MAX_CHANGES];

/* helper function for timing tolerance */
static inline unsigned int diff(int A, int B) {
	return abs(A - B);
}

static bool decode_digoo(unsigned int changeCount)
{
	unsigned long code = 0;
	const unsigned int T = timings[0] / 8;
	const unsigned int delayTolerance = T * nReceiveTolerance / 100;
	char buf[64];
	int i;
	int16_t temp;

	/* We need a start gap + 36 bits */
	if (changeCount < 73)
		return false;

	/* Inter-transmission gap is ~ 8T */
	if (timings[0] < 3500)
		return false;

	for (i = 1; i < changeCount - 1; i += 2) {
		/* First pulse should be duration T */
		if (diff(timings[i], T) > delayTolerance)
			return false;

		if (i > 48 && i < 57) {
			/* 4 bits of 1 (4T) in bits 25->28 */
			if (diff(timings[i + 1], 4 * T) > delayTolerance)
				return false;
		} else {
			code <<= 1;

			if (diff(timings[i + 1], 2 * T) < delayTolerance) {
				/* Zero is 2T */
			} else if (diff(timings[i + 1], 4 * T) <
					delayTolerance) {
				/* One is 4T */
				code |= 1;
			} else {
				/* Out of spec for what we want */
				return false;
			}
		}
	}

	/* If we get this far we have valid Digoo data in 'code' */
	/* IIIIIIII B0CCTTTT TTTTTTTT HHHHHHHH */
	if (uart_lock()) {
		sprintf(buf, "ID=%u CH=%u BAT=%s",
				(unsigned char) (code >> 24),
				(unsigned char) ((code >> 20) & 0x3) + 1,
				((code >> 20) & 8) ? "OK" : "LOW");
		uart_puts(buf);
		temp = (code >> 8) & 0xFFF;
		temp |= (temp & 0x800) ? 0xF000 : 0;
		sprintf(buf, " T=%d.%d",
				temp / 10,
				abs(temp % 10));
		uart_puts(buf);
		/* Humidity is apparently optional on some sensors */
		if (code & 0xFF) {
			sprintf(buf, " H=%u", (unsigned char) (code & 0xFF));
			uart_puts(buf);
		}
		/* Output the raw code to aid debugging */
		sprintf(buf, " R=%08lX", code);
		uart_puts(buf);
		uart_puts("\r\n");
		uart_unlock();
	}

	return true;
}

/*
 * Triggered by the 433MHz receiver (PB0)
 */
ISR(PCINT0_vect)
{
	static unsigned long lastTime = 0;
	static unsigned int changeCount = 0;
	static unsigned int repeatCount = 0;

	const long time = micros();
	const unsigned int duration = time - lastTime;

	if (duration > nSeparationLimit) {
		if (diff(duration, timings[0]) < 400) {
			repeatCount++;
			if ((repeatCount == 2) && decode_digoo(changeCount)) {
				repeatCount = 0;
			}
		}
		changeCount = 0;
	}

	/* Check for overflow */
	if (changeCount >= MAX_CHANGES) {
		changeCount = 0;
		repeatCount = 0;
	}

	timings[changeCount++] = duration;
	lastTime = time;
}

void r433_init(void)
{
	/* Set the receiver bit to input mode */
	DDRB &= ~(1 << RECEIVER_PIN);
	/* Enable interrupts from receiver on pin change */
	PCMSK = (1 << RECEIVER_PIN);
	GIMSK |= _BV(PCIE);
}
