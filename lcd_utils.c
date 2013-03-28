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

#include "lcd.h"
#include "lcd_utils.h"

static UINT8 iPrevHour = 0, iPrevMin = 0;

static void display_sub_time (int iNumbers, int iIndex)
{
	if (iNumbers < 10)
	{
		char cBuff[1];
		itoa (iNumbers, cBuff, 10); // convert to char
		lcd_save_char (iIndex, '0');
		lcd_save_char (iIndex + 1, cBuff[0]);
	}
	else
		lcd_save_word (iIndex, iNumbers);
}
 
void display_ftime (long fTime, BYTE bFullRefresh)
{ // t est le temps transforme en seconde 3600 * H + 60 * M + S
	int iHour = fTime / 3600, iMin = (fTime / 60) % 60, iSec = fTime % 60;

	if (bFullRefresh || iHour != iPrevHour)
	{
		display_sub_time (iHour, POSITION_HOURS);
		iPrevHour = iHour;
	}

	// lcd_display_char (18, ':');


	if (bFullRefresh || iMin != iPrevMin)
	{
		display_sub_time (iMin, POSITION_MINUTES);
		iPrevMin = iMin;
	}

	// lcd_display_char (21, ':');

	// if (iSec != iPrevSec)
	display_sub_time (iSec, POSITION_SECONDS);

	lcd_update_display ();
}
