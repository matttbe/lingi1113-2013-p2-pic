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

#define __18CXX // => pic18
#include <pic18fregs.h>

#define LOW(a)     (a & 0xFF)
#define HIGH(a)    ((a>>8) & 0xFF)
#define CLOCK_FREQ 25000000 // 25MHz
#define EXEC_FREQ (CLOCK_FREQ / 4)  //4 clock cycles to execute 1 instruction

#define CLOCK_NAME "Tea Time Clock"

#include "delay.c"

typedef enum {
	BUTTON_CLOCK = 0,
	BUTTON_HOURS_CLOCK,
	BUTTON_MINUTES_CLOCK,
	BUTTON_SECONDS_CLOCK,
	BUTTON_HOURS_ALARM,
	BUTTON_MINUTES_ALARM,
	BUTTON_SECONDS_ALARM
} ButtonState;

ButtonState iState = 0;
UINT32 fTime = 0, fAlarm = 0;
UINT8 iHours = 0, iMinutes = 0, iSeconds = 0; // Réglage de l'heure de l'horloge et de l'fAlarme
UINT8 iBip = 0;

static UINT32 _get_time (UINT8 iH, UINT8 iM, UINT8 iS)
{
	UINT32 fCurrTime = 3600 * (long int)iH + 60 * (long int)iM + (long int)iS;
	return fCurrTime;
}

// high interruptions
void high_isr (void) __interrupt 1
{
	if(INTCON3bits.INT1IF == 1 || INTCON3bits.INT3IF == 1)
	{
		long int fCurrentTime;
		if(BUTTON0_IO)
		{ // change the clock/alarm
			UINT8 bRefresh = 1;
			switch (iState)
			{
				case BUTTON_HOURS_CLOCK : // hour
				case BUTTON_HOURS_ALARM :
					iHours = (iHours + 1) % 24;
				break;
				case BUTTON_MINUTES_CLOCK : // min
				case BUTTON_MINUTES_ALARM :
					iMinutes = (iMinutes + 1) % 60;
				break;
				case BUTTON_SECONDS_CLOCK : // sec
				case BUTTON_SECONDS_ALARM :
					iSeconds = (iSeconds + 1) % 60;
				break;
				default: // horloge: state == 0
					bRefresh = 0;
				break;
			}
			if (bRefresh)
			{
				fCurrentTime = _get_time (iHours, iMinutes, iSeconds);
				display_ftime (fCurrentTime, 1); // display the time here in the interruption (we don't check the seconds)
			}

			INTCON3bits.INT1IF = 0;   //clear INT1 flag
			INTCON3bits.INT3IF = 0; 
			INTCONbits.T0IF = 0;
		}

		else if (BUTTON1_IO)
		{ // Bouton pour passer à l'état suivant
			iState = (iState + 1) % 7;
			switch (iState)
			{
				case BUTTON_HOURS_CLOCK :
					iHours = fTime / 3600;
					iMinutes = (fTime / 60) % 60;
					iSeconds = fTime % 60;
					lcd_display_string (0, "SetClock : hour");
				break;
				case BUTTON_MINUTES_CLOCK :
					lcd_display_string (0, "SetClock : min");
				break;
				case BUTTON_SECONDS_CLOCK :
					lcd_display_string (0, "SetClock : sec");
				break;
				case BUTTON_HOURS_ALARM :
					fTime = _get_time (iHours, iMinutes, iSeconds); // the new time is now defined
					iHours = fAlarm / 3600;
					iMinutes = (fAlarm / 60) % 60;
					iSeconds = fAlarm % 60;

					lcd_display_string (0, "SetfAlarm : hour");
					lcd_display_string (POSITION_HOURS, "  :  :  ");
				break;
				case BUTTON_MINUTES_ALARM :
					lcd_display_string (0, "SetfAlarm : min");
				break;
				case BUTTON_SECONDS_ALARM :
					lcd_display_string (0, "SetfAlarm : sec");
				break;
				case BUTTON_CLOCK :
					fAlarm = _get_time (iHours, iMinutes, iSeconds); // the alarm is now defined
					lcd_display_string (0, CLOCK_NAME);
					lcd_display_string (POSITION_HOURS, "  :  :  ");
				break;
				default:
				break;
			}
			fCurrentTime = _get_time (iHours, iMinutes, iSeconds);
			display_ftime (fCurrentTime, 1); // display the time here in the interruption (we don't check the seconds)
		}

		INTCON3bits.INT1IF  = 0;
		INTCON3bits.INT3IF  = 0;
		INTCONbits.T0IF     = 0;
	}
}


void main (void)
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

	lcd_display_string (0, CLOCK_NAME); //first arg is start position
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

