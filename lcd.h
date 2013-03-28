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

#ifndef __LCD_H__
#define __LCD_H__

#include "Include/GenericTypeDefs.h"

void lcd_save_word (BYTE iPos, WORD iWord);

void lcd_display_word (BYTE iPos, WORD iWord);

/*************************************************
 Function lcd_display_string: 
 Writes an IP address to string to the LCD display
 starting at pos
*************************************************/
void lcd_display_string (BYTE iPos, char* cText);

void lcd_save_char (BYTE iPos, char cChar);

void lcd_display_char (BYTE iPos, char cChar);

void lcd_update_display (void);

#endif

