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

static void display_sub_time (BYTE iNumbers, BYTE iIndex)
{
	if (iNumbers < 10)
	{
		char cBuff[1];
		sprintf (cBuff, "%hd", iNumbers); // convert to char
		lcd_save_char (iIndex, '0');
		lcd_save_char (iIndex + 1, cBuff[0]);
	}
	else
	{
		char cBuff[2];
		sprintf (cBuff, "%hd", iNumbers); // convert to char
		lcd_save_char (iIndex, cBuff[0]);
		lcd_save_char (iIndex + 1, cBuff[1]);
	}
}
 
void display_ftime (long int fCurrentTime, BYTE bFullRefresh)
{ // t est le temps transforme en seconde 3600 * H + 60 * M + S
	BYTE iHour = fCurrentTime / 3600, iMin = (fCurrentTime / 60) % 60, iSec = fCurrentTime % 60;

	if (bFullRefresh || iHour != iPrevHour)
	{
		display_sub_time (iHour, POSITION_HOURS);
		iPrevHour = iHour;
	}

	if (bFullRefresh || iMin != iPrevMin)
	{
		display_sub_time (iMin, POSITION_MINUTES);
		iPrevMin = iMin;
	}

	display_sub_time (iSec, POSITION_SECONDS);

	lcd_update_display ();
}
