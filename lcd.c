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

#include <stdlib.h>

#include "lcd.h"
#include "strlcpy.h"

/*************************************************
 Function lcd_display_word:
 writes a WORD in hexa on the position indicated by
 pos. 
 - pos=0 -> 1st line of the LCD
 - pos=16 -> 2nd line of the LCD

 __SDCC__ only: for debugging
*************************************************/
#if defined(__SDCC__)
void lcd_save_word (BYTE iPos, WORD iWord) //WORD is a 16 bits unsigned
{
	BYTE WDigit[6]; //enough for a  number < 65636: 5 digits + \0
	BYTE j;
	BYTE LCDPos = 0;  //write on first line of LCD
	unsigned radix = 10; //type expected by sdcc's ultoa()

	LCDPos = iPos;
	ultoa (iWord, WDigit, radix);
	for (j = 0; j < strlen ((char*) WDigit); j++)
	{
		LCDText[LCDPos++] = WDigit[j];
	}
	if (LCDPos < 32u)
		LCDText[LCDPos] = 0;
}

void lcd_display_word (BYTE iPos, WORD iWord)
{
	lcd_save_word (iPos, iWord);
	lcd_update_display ();
}
/*************************************************
 Function lcd_display_string: 
 Writes an IP address to string to the LCD display
 starting at pos
*************************************************/
void lcd_display_string (BYTE iPos, char* cText)
{
	BYTE iLength = strlen (cText) + 1;
	BYTE iMax = 32 - iPos;
	strlcpy ((char*) &LCDText[iPos], cText, (iLength < iMax) ? iLength : iMax);
	lcd_update_display ();
}

void lcd_save_char (BYTE iPos, char cChar)
{
	LCDText[iPos] = cChar;
}

void lcd_display_char (BYTE iPos, char cChar)
{
	lcd_save_char (iPos, cChar);
	lcd_update_display ();
}

void lcd_update_display (void)
{
	LCDUpdate ();
}
#endif

