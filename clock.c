#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/LCDBlocking.h"
#include "Include/TCPIP_Stack/Delay.h"

#include "lcd.c"
#include "lcd_utils.c"
#include "strlcpy.c"

#define __18CXX          //means "any pic18"
#include <pic18fregs.h>  //defines the address corresponding to the symbolic
                         //names of the sfr

#define LOW(a)     (a & 0xFF)
#define HIGH(a)    ((a>>8) & 0xFF)
#define CLOCK_FREQ 25000000     // Fréquence modifiée pour correspondre à une seconde
#define EXEC_FREQ CLOCK_FREQ/4  //4 clock cycles to execute 1 instruction

void delay_1ms (void);
void delay_ms (unsigned int ms);


UINT8 iState = 0;
UINT32 fTime = 0, fAlarm = 0;
UINT8 iHours = 0, iMinutes = 0, iSeconds = 0; // Réglage de l'heure de l'horloge et de l'fAlarme
UINT8 bBool = 0; // Assure qu'on entre dans la routine que d'un seul état par interruption
UINT8 i, iBip = 0;


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
				iHours = (iHours + 1) % 24;
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if((iState == 2  || iState == 5) && bBool== 1)
			{ // Reglage minutes Horloge
				iMinutes = (iMinutes + 1) % 60 ;
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if((iState == 3  || iState == 6) && bBool== 1)
			{ // Reglage secondes Horloge
				iSeconds = (iSeconds + 1) % 60;
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}


			INTCON3bits.INT1IF = 0;   //clear INT1 flag
			INTCON3bits.INT3IF = 0; 
			INTCONbits.T0IF=0;
		}

		else if(BUTTON1_IO)
		{ // Bouton pour passer à l'état suivant
			if(iState == 0  && bBool == 1)
			{ 
				iState = 1;
				iHours = fTime / 3600;
				iMinutes = (fTime / 60) % 60;
				iSeconds = fTime % 60;
				lcd_display_string (0, "SetClock : hour");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if(iState == 1 && bBool == 1)
			{
				iState = 2;
				lcd_display_string (0, "SetClock : min");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if(iState == 2 && bBool == 1)
			{
				iState = 3;
				lcd_display_string (0, "SetClock : sec");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if(iState == 3 && bBool == 1)
			{
				iState = 4;
				fTime = 3600 * iHours + 60 * iMinutes + iSeconds;
				iHours = fAlarm / 3600;
				iMinutes = (fAlarm / 60) % 60;
				iSeconds = fAlarm % 60;

				lcd_display_string (0, "SetfAlarm : hour");
				lcd_display_string (POSITION_HOURS, "00:00:00");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if(iState == 4 && bBool== 1)
			{
				iState = 5;
				lcd_display_string (0, "SetfAlarm : min");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if(iState == 5 && bBool== 1)
			{
				iState = 6;
				lcd_display_string (0, "SetfAlarm : sec");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
				bBool = 0;
			}

			if(iState == 6 && bBool== 1)
			{
				iState = 0;
				fAlarm = 3600 * iHours + 60 * iMinutes + iSeconds;

				lcd_display_string (0, "THE TIME MACHINE");
				lcd_display_string (POSITION_HOURS, "00:00:00");
				display_ftime (3600 * iHours + 60 * iMinutes + iSeconds, 1);
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

	lcd_display_string (0, "THE TIME MACHINE"); //first arg is start position
	// on 32 positions LCD
	lcd_display_string (POSITION_HOURS, "00:00:00");

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
			display_ftime(fTime, 0);
		}
	}
 
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

void delay_ms (unsigned int ms)
{
	while (ms--)
	{
		delay_1ms ();
	}
}

