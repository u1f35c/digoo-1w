/*
 * Basic firmware to test timer + serial TX.
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
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include <avr/pgmspace.h>

#include "timer.h"
#include "tx_uart.h"

void idle(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}

int __attribute__((noreturn)) main(void)
{
	unsigned long last_time, cur_time;

	/* We don't need the ADC, so power it down */
	ACSR |= (1 << ACD);

	timer_init();
	uart_init();

	DDRB |= 1 << PB1;	/* PB1 output for LED */

	sei(); /* We're ready to go; enable interrupts */

	uart_puts("Hello World!\r\n");

	last_time = 0;
	while (1) {
		cur_time = millis();
		if ((cur_time - last_time) > 60000) {
			uart_puts("Another minute\r\n");
			last_time = cur_time;
		}
		/* Idle the CPU; the timer interrupt and/or pin change will wake us */
		idle();
	}
}
