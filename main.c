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
#include <stdio.h>
#include <util/delay.h>

#include <avr/pgmspace.h>

#include "r433.h"
#include "timer.h"
#include "tx_uart.h"
#include "w1.h"

char serial[17];

inline char hexdigit(unsigned int i)
{
	return (i < 10) ? ('0' + i) : ('A' - 10 + i);
}

/* Look for a 1-Wire device and use its ROMID to set the serial ID */
static void read_serial(void)
{
	uint8_t buf[8];
	uint8_t i;

	if (!w1_reset(false)) {
		serial[0] = 0;
		return;
	}

	w1_write(0x33);		/* READ ROM */
	w1_read(buf, 8);

	for (i = 0; i < 8; i++) {
		serial[i * 2] = hexdigit(buf[i] >> 4);
		serial[i * 2 + 1] = hexdigit(buf[i] & 0xF);
	}
	serial[16] = 0;
}

static void read_temperature(void)
{
	int i;
	uint8_t buf[9];
	char tmp[5];

	if (!w1_reset(false)) {
		return;
	}

	w1_write(0xCC);		/* SKIP ROM */
	w1_write(0x44);		/* Convert T */

	while (!w1_read_bit())
		;

	if (!w1_reset(false)) {
		return;
	}
	w1_write(0xCC);		/* SKIP ROM */
	w1_write(0xBE);		/* Read Scratchpad */

	for (i = 0; i < 9; i++) {
		buf[i] = w1_read_byte();
	}

	if (uart_lock()) {
		uart_puts("ID=");
		uart_puts(serial);
		uart_puts(" T=");
		if (buf[1] & 0x80)
			uart_tx('-');
		sprintf(tmp, "%d", (buf[1] & 0x7) << 4 | (buf[0] >> 4));
		uart_puts(tmp);
		i = sprintf(tmp, "%04d", 625 * (buf[0] & 0xF));
		uart_tx('.');
		uart_puts(tmp);
		uart_puts("\r\n");
		uart_unlock();
	}
}

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
	r433_init();
	w1_init();

	DDRB |= 1 << PB1;	/* PB1 output for LED */

	sei(); /* We're ready to go; enable interrupts */

	read_serial();
	if (uart_lock()) {
		uart_puts("\r\nN=digoo-w1 V=0.1 ID=");
		uart_puts(serial);
		uart_puts("\r\n");
		uart_unlock();
	}

	last_time = 0;
	while (1) {
		cur_time = millis();
		if ((cur_time - last_time) > 60000) {
			read_temperature();
			last_time = cur_time;
		}
		/* Idle the CPU; the timer interrupt and/or pin change will wake us */
		idle();
	}
}
