#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/LCDBlocking.h"
#include "Include/TCPIP_Stack/Delay.h"

#define __18CXX          //means "any pic18"
#include <pic18fregs.h>  //defines the address corresponding to the symbolic
                         //names of the sfr

#define LOW(a)     (a & 0xFF)
#define HIGH(a)    ((a>>8) & 0xFF)
#define CLOCK_FREQ 25000000     // Fréquence modifiée pour correspondre à une seconde
#define EXEC_FREQ CLOCK_FREQ/4  //4 clock cycles to execute 1 instruction

void DisplayString (BYTE pos, char* text);
void DisplayWORD (BYTE pos, WORD w);
void DisplayIPValue (DWORD IPdw);
size_t strlcpy (char *dst, const char *src, size_t siz);
void delay_1ms (void);
void delay_ms (unsigned int ms);
void DisplayChar (BYTE pos, char text);
void DisplayfTime (long t);


int iState = 0;
long fTime = 0, fAlarm = 0;
long fHour = 0, fMinutes = 0, fSeconds = 0; // Réglage de l'heure de l'horloge et de l'fAlarme
int iPrevHour = 0, iPrevMin = 0, iPrevSec = 0;
int bBool = 0; // Assure qu'on entre dans la routine que d'un seul état par interruption
int i, iBip = 0;


void high_isr (void) interrupt 1
{
	bBool = 1;
	if(INTCON3bits.INT1IF == 1 || INTCON3bits.INT3IF == 1)
	{
		if(BUTTON0_IO)
		{ // Bouton pour régler l'heure
			if(iState == 0 && bBool== 1)
			{ // Etat Horloge
				bBool = 0;
			}

			if((iState == 1 || iState == 4) && bBool== 1)
			{ // Reglage heure Horloge
				fHour = (fHour + 1) % 24;
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if((iState == 2  || iState == 5) && bBool== 1)
			{ // Reglage minutes Horloge
				fMinutes = (fMinutes + 1) % 60 ;
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if((iState == 3  || iState == 6) && bBool== 1)
			{ // Reglage secondes Horloge
				fSeconds = (fSeconds + 1) % 60;
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}


			INTCON3bits.INT1IF = 0;   //clear INT1 flag
			INTCON3bits.INT3IF = 0; 
			INTCONbits.T0IF=0;
		}

		if(BUTTON1_IO)
		{ // Bouton pour passer à l'état suivant
			if(iState == 0  && bBool == 1)
			{ 
				iState = 1;
				fHour = fTime / 3600;
				fMinutes = (fTime / 60) % 60;
				fSeconds = fTime % 60;
				DisplayString (0, "SetClock : hours");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if(iState == 1 && bBool == 1)
			{
				iState = 2;
				DisplayString (0, "SetClock : min");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if(iState == 2 && bBool == 1)
			{
				iState = 3;
				DisplayString (0, "SetClock : sec");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if(iState == 3 && bBool == 1)
			{
				iState = 4;
				fTime = 3600 * fHour + 60 * fMinutes + fSeconds;
				fHour = fAlarm / 3600;
				fMinutes = (fAlarm / 60) % 60;
				fSeconds = fAlarm % 60;

				DisplayString (0, "SetfAlarm : hours");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if(iState == 4 && bBool== 1)
			{
				iState = 5;
				DisplayString (0, "SetfAlarm : min");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if(iState == 5 && bBool== 1)
			{
				iState = 6;
				DisplayString (0, "SetfAlarm : sec");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}

			if(iState == 6 && bBool== 1)
			{
				iState = 0;
				fAlarm = 3600 * fHour + 60 * fMinutes + fSeconds;

				DisplayString (0, "THE TIME MACHINE");
				DisplayfTime (3600 * fHour + 60 * fMinutes + fSeconds);
				bBool = 0;
			}
		}

		INTCON3bits.INT1IF  = 0;  
		INTCON3bits.INT3IF  = 0; 
		INTCONbits.T0IF     = 0; 
	}
}


void main(void)
{
	LED0_TRIS = 0; //configure 1st led pin as output (yellow)

	BUTTON0_TRIS = 1; //configure button0 as input
	BUTTON1_TRIS = 1; //configure button0 as input

	LED0_IO = 0;

	RCONbits.IPEN       = 1;   //enable interrupts priority levels
	INTCONbits.GIE      = 1;   //enable high priority interrupts

	INTCON3bits.INT1IP  = 1;   //connect INT1 interrupt (button 2) to high po
	INTCON2bits.INTEDG1 = 0;   //INT1 interrupts on raising edge
	INTCON3bits.INT1IE  = 1;   //enable INT1 interrupt (button 2)
	INTCON3bits.INT1IF  = 0;   //clear INT1 flag

	INTCON2bits.INT3IP  = 1;   // Idem mais pour la 2eme interruption
	INTCON2bits.INTEDG3 = 0;   
	INTCON3bits.INT3IE  = 1;   
	INTCON3bits.INT3IF  = 0;   



	LCDInit ();
	delay_ms (1000);

	DisplayString (0, "THE TIME MACHINE"); //first arg is start position
	// on 32 positions LCD
	DisplayString (16, "00:00:00");

	iState = 0;

	while(1)
	{
		delay_ms (1000);
		if (iState == 0)
		{
			fTime++;
			fTime = fTime % 86400; // Car on incrémente toujours l'heure, il est possible qu'on atteigne une heure > 24:00:00, d'où le modulo
			if (fTime == fAlarm) // Heure de l'fAlarme
				iBip = 30;

			if(iBip > 0)
			{ // On fait cligonter pendant 30 secondes en cas d'fAlarme
				LED0_IO ^= 1;
				iBip--;
			}
			DisplayfTime(fTime);
		}
	}
 
}

static void display_sub_time (int iNumbers, int iIndex)
{
	if (iNumbers < 10)
	{
		DisplayWORD (iIndex, 0);
		DisplayWORD (iIndex + 1, iNumbers);
	}
	else
		DisplayWORD (iIndex, iNumbers);
}
 
void DisplayfTime (long t)
{ // t est le temps transforme en seconde 3600 * H + 60 * M + S
	int iHour = t / 3600, iMin = (t / 60) % 60, iSec = t % 60;

	if (iHour != iPrevHour)
		display_sub_time (iHour, 16);

	// DisplayChar (18, ':');


	if (iMin != iPrevMin)
		display_sub_time (iMin, 19);

	// DisplayChar (21, ':');

	if (iSec != iPrevSec)
		display_sub_time (iSec, 22);
}
/*************************************************
 Function DisplayWORD:
 writes a WORD in hexa on the position indicated by
 pos. 
 - pos=0 -> 1st line of the LCD
 - pos=16 -> 2nd line of the LCD

 __SDCC__ only: for debugging
*************************************************/
#if defined(__SDCC__)
void DisplayWORD (BYTE pos, WORD w) //WORD is a 16 bits unsigned
{
	BYTE WDigit[6]; //enough for a  number < 65636: 5 digits + \0
	BYTE j;
	BYTE LCDPos = 0;  //write on first line of LCD
	unsigned radix = 10; //type expected by sdcc's ultoa()

	LCDPos = pos;
	ultoa (w, WDigit, radix);      
	for (j = 0; j < strlen ((char*) WDigit); j++)
	{
		LCDText[LCDPos++] = WDigit[j];
	}
	if (LCDPos < 32u)
		LCDText[LCDPos] = 0;

	LCDUpdate ();
}
/*************************************************
 Function DisplayString: 
 Writes an IP address to string to the LCD display
 starting at pos
*************************************************/
void DisplayString(BYTE pos, char* text)
{
	BYTE l = strlen (text) + 1;
	BYTE max = 32 - pos;
	strlcpy ((char*) &LCDText[pos], text, (l<max) ? l : max);
	LCDUpdate();
}
#endif


/*-------------------------------------------------------------------------
 *
 * strlcpy.c
 *    strncpy done right
 *
 * This file was taken from OpenBSD and is used on platforms that don't
 * provide strlcpy().  The OpenBSD copyright terms follow.
 *-------------------------------------------------------------------------
 */

/*  $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $    */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 * Function creation history:  http://www.gratisoft.us/todd/papers/strlcpy.html
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
    char       *d = dst;
    const char *s = src;
    size_t      n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0)
    {
        while (--n != 0)
        {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';          /* NUL-terminate dst */
        while (*s++)
            ;
    }



    return (s - src - 1);       /* count does not include NUL */
}

void DisplayChar (BYTE pos, char text)
{
	LCDText[pos] = text;
	LCDUpdate ();
} 

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

void delay_ms(unsigned int ms)
{
	while (ms--)
	{
		delay_1ms ();
	}
}

