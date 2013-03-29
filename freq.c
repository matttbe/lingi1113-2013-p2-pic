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
#define INIT_TIMER0H 0x49
#define INIT_TIMER0L 0x00

#include "delay.c"

UINT8 bRefresh = 0, iInitTmr0 = 0x49;
// interruption counter: all / timer0 / timer1
UINT16 fInterruptTmr = 0, fInterruptTmr0 = 0, fInterruptTmr1 = 0;

static void _clean_counter_and_refresh (void)
{
	fInterruptTmr0 = 0;
	fInterruptTmr1 = 0;
	bRefresh = 1;
}

// high interruptions
void high_isr (void) __interrupt 1
{
	if (INTCONbits.TMR0IF == 1) // overflow on TMR0
	{
		fInterruptTmr0++;
		
		TMR0H = iInitTmr0; // restart the counter for TMR0H
		TMR0L = INIT_TIMER0L; // same for low bits
		INTCONbits.TMR0IF = 0;//clear timer0 interrupt flag
	}

	else if (PIR1bits.TMR1IF == 1) // overflow on TMR0
	{
		fInterruptTmr1++; // each 2 secondes (sure)
		fInterruptTmr += 2; // just to stop the initialisation

		if (fInterruptTmr % 10) // each x sec
		{
			UINT16 fDiff = fInterruptTmr0 - (fInterruptTmr1*2);
			if (fDiff > 0)
			{
				iInitTmr0--; // the timer is faster
				_clean_counter_and_refresh ();
			}
			else if (fDiff < 0)
			{
				iInitTmr0++; // the timer is slower
				_clean_counter_and_refresh ();
			}
		}

		PIR1bits.TMR1IF = 0; // reset overflow signal
	}
}


void main (void)
{
	// LCD
	LCDInit ();
	delay_ms (100);

	lcd_display_string (0, "INITIALIZATION  IN PROGRESS..."); //first arg is start position
	// on 32 positions LCD

	RCONbits.IPEN       = 1;   //enable interrupts priority levels
	INTCONbits.GIE      = 1;   //enable high priority interrupts

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

	//init timer1
	TMR1L = 0x00;           // 2 sec per interruption
	T1CONbits.TMR1ON  = 0;  // same as timer 0
	T1CONbits.TMR1CS  = 1;
	T1CONbits.T1SYNC  = 1;
	T1CONbits.T1OSCEN = 1;
	T1CONbits.T1CKPS1 = 0;
	T1CONbits.T1CKPS0 = 0;
	PIE1bits.TMR1IE   = 1;  // enable interrupts for timer 1

	T0CONbits.TMR0ON  = 1;  //enable timer0
    T1CONbits.TMR1ON  = 1;  //enable timer1

	while (1)
	{
		if (bRefresh == 1)
		{
			char cText[32];// = {0};
			sprintf (cText, "Init Timer:     %hd        ", iInitTmr0);
			lcd_display_string (0, cText);
			bRefresh = 0;
		}
	}
}

