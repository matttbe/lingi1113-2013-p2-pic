/*
 * Project P2
 * 
 * Copyright 2013 UCLouvain / pole INGI
 *
 * Authors:
 *   Matthieu Baerts <matthieu.baerts@student.uclouvain.be>
 *   Hélène Verhaeghe <helene.verhaeghe@student.uclouvain.be>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

void delay_1ms (void)
{
	TMR0H = (0x10000 - EXEC_FREQ / 1000) >> 8;
	TMR0L = (0x10000 - EXEC_FREQ / 1000) & 0xff;
	T0CONbits.TMR0ON = 0; // disable timer
	T0CONbits.T08BIT = 0; // use timer 16-bit counter
	T0CONbits.T0CS   = 0; // use timer instruction cycle clock
	T0CONbits.PSA    = 1; // disable timer prescaler
	INTCONbits.T0IF  = 0; // clear timer overflow bit
	T0CONbits.TMR0ON = 1; // enable timer
	while (! INTCONbits.T0IF) {} // wait for timer overflow
	INTCONbits.T0IF  = 0; // clear timer overflow bit
	T0CONbits.TMR0ON = 0; // disable timer
}

void delay_ms (unsigned int ms)
{
	while (ms--)
	{
		delay_1ms ();
	}
}
