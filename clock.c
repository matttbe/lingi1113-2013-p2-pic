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
#define INIT_TIMER0H 0x45 // 69 in decimals
#define INIT_TIMER0L 0x00

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

ButtonState iState = BUTTON_CLOCK;
UINT32 fTime = 0, fAlarm = 0;
UINT8 iHours = 0, iMinutes = 0, iSeconds = 0; // Réglage de l'heure de l'horloge et de l'fAlarme
UINT8 iBip = 0;
UINT8 bRefresh = 0;

static UINT32 _get_time (UINT8 iH, UINT8 iM, UINT8 iS)
{
	UINT32 fCurrTime = 3600 * (long int)iH + 60 * (long int)iM + (long int)iS;
	return fCurrTime;
}

// high interruptions
void high_isr (void) __interrupt 1
{
	if (INTCONbits.TMR0IF == 1 // overflow on TMR0
		&& (iState == 0 || iState > BUTTON_SECONDS_CLOCK)) // only for the clock
	{
		fTime++;
		fTime = fTime % 86400; // < 24h
		if (fTime == fAlarm) // Bomb exploded
			iBip = 30;

		if(iBip > 0)
		{ // the led can light during 30sec
			LED0_IO ^= 1;
			iBip--;
		}
		// display_ftime(fTime, 0); // display not in the interruption but in the mainloop
		bRefresh = 1;
		
		TMR0H = INIT_TIMER0H; // restart the counter for TMR0H
		TMR0L = INIT_TIMER0L; // same for low bits
		INTCONbits.TMR0IF = 0;//clear timer0 interrupt flag
	}
	if (INTCON3bits.INT1IF == 1 || INTCON3bits.INT3IF == 1) // buttons
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
					lcd_display_string (0, "SetClock: hour");
				break;
				case BUTTON_MINUTES_CLOCK :
					lcd_display_string (0, "SetClock: min");
				break;
				case BUTTON_SECONDS_CLOCK :
					lcd_display_string (0, "SetClock: sec");
				break;
				case BUTTON_HOURS_ALARM :
					fTime = _get_time (iHours, iMinutes, iSeconds); // the new time is now defined
					iHours = fAlarm / 3600;
					iMinutes = (fAlarm / 60) % 60;
					iSeconds = fAlarm % 60;

					lcd_display_string (0, "SetfAlarm: hour");
					lcd_display_string (POSITION_HOURS, "  :  :  ");
				break;
				case BUTTON_MINUTES_ALARM :
					lcd_display_string (0, "SetfAlarm: min");
				break;
				case BUTTON_SECONDS_ALARM :
					lcd_display_string (0, "SetfAlarm: sec");
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
			display_ftime (fCurrentTime, 1); // display the time here in the interruption because the time stopped here
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

	// LCD
	LCDInit ();
	delay_ms (100);

	RCONbits.IPEN       = 1;   //enable interrupts priority levels
	INTCONbits.GIE      = 1;   //enable high priority interrupts

	INTCON3bits.INT1IP  = 1;   //connect INT1 interrupt (button 2) to high po
	INTCON2bits.INTEDG1 = 0;   //INT1 interrupts on raising edge
	INTCON3bits.INT1IE  = 1;   //enable INT1 interrupt (button 2)
	INTCON3bits.INT1IF  = 0;   //clear INT1 flag

	INTCON2bits.INT3IP  = 1;   // same thing for other button
	INTCON2bits.INTEDG3 = 0;
	INTCON3bits.INT3IE  = 1;
	INTCON3bits.INT3IF  = 0;

	// init timer0
	TMR0H = INIT_TIMER0H;   // init timer0 => 70 from freq.c
	TMR0L = 0x00;           // low bits
	T0CONbits.TMR0ON  = 0;  // we will enable it after
	T0CONbits.T08BIT  = 0;  // timer0 16-bit counter
	T0CONbits.T0CS    = 0;  // timer0 instruction cycle clock
	T0CONbits.PSA     = 0;  // timer0 prescaler: 0 <=> enable
	INTCONbits.T0IF   = 0;  // clear timer0 overflow bit
	INTCONbits.TMR0IE = 1;  // enable interrupts for timer 0
	T0CONbits.T0PS0   = 0;  // prescale 128
	T0CONbits.T0PS1   = 1;
	T0CONbits.T0PS2   = 1;

	T0CONbits.TMR0ON = 1;  //enable timer0

	lcd_display_string (0, CLOCK_NAME); //first arg is start position
	// on 32 positions LCD
	lcd_display_string (POSITION_HOURS, "00:00:00");

	while (1)
	{
		if (bRefresh == 1 && iState == 0)
		{
			display_ftime(fTime, 0); // do not refresh the lcd in the interruption
			bRefresh = 0;
		}
	}
}

