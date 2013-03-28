/******************************************************************************
	fichier freq.c

auteurs :
---------
    JeSaispasQui
	fichier du base dans le dossier du cours
    Guillaume Maudoux
	guillaume.maudoux@student.uclouvain.be
    Simon Claessens
	simon.claessens@student.uclouvain.be
	Gaetan Collart
	gaetan.clollart@student.uclouvain.be

Ce fichier implémente un pic rudimentaire sur une plaque de dévelloppement
Pic-Maxi-Web (r)
munie d'un microcontrolleur PIC de type 18F97J60


utilisation :
-------------
 - affiche la fréquence du timer0 calculée grace au timer1

ATTENTION : 

Il est probable que le fichier ne compile pas, 
et même dans ce cas, il se peut que le résultat ne soit pas exact sur un pic.

Merci de votre compréhension.


Copiez-le, partagez-le, déchirez-le... peu m'importe.

*******************************************************************************/

#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/LCDBlocking.h"
#include "Include/TCPIP_Stack/Delay.h"

#define LOW(a)     (a & 0xFF)
#define HIGH(a)    ((a>>8) & 0xFF)
#define CLOCK_FREQ 25000000     //25 MHz
// Par contre il faut bien 4 cycles / instruction, 
// et aussi 4 cycles avant d'augmenter le timer de 1.
#define EXEC_FREQ CLOCK_FREQ/4  //4 clock cycles to execute 1 instruction

// definit deux alias tres utiles
// si le bit xxxxF est à 1 ca veut dire que l'interruption
// xxxx est active.
#define INT1F INTCON3bits.INT1F // Interruption INT1 : généré par le bouton
#define TMR0F INTCONbits.TMR0IF // Int TMR0, généré par le timer
#define TMR1F PIR1bits.TMR1IF // Int TMR1, généré par le timer

// signature des fonctions
void DisplayString(BYTE pos, char* text);
void DisplayWORD(BYTE pos, WORD w);
void DisplayIPValue(DWORD IPdw);
size_t strlcpy(char *dst, const char *src, size_t siz);
void dumb_delay1ms (void);

// Variables - Globales comme ca on peut les utiliser dans
// la routine d'interruption...
long freq = 0;
long interrT0 = 0;
long interrT1 = 0;

// Routine d'interruption. 
// On la reconnait au mot cle __interrupt X
// Ici X vaut 1, on définit donc la routine appelée pour les interruption
// hautes, ou importantes.

// Il existe deux autres type : 
// 0 : Interruption de RESET, on ne peut pas la redéfinir. 
//	Elle est par exemple appélée lorsque on appuye sur le bouton rouge
// 2 : interruption basses, ou faibles. ces interruptions peuvent être 
//     a leur tour interrompues par des interruptions hautes.

// Nous n'utilisons que les interruptions hautes.
void high_isr (void) __interrupt 1 
{
	char freqString[32] = {0};
    //Timer 1 en 8 bits a 10MHz genère 78125 interruption par 2 sec
    
    if ( TMR1F ){        
		interrT1++;
		if ((interrT1 % 78125) == 0){// 1) == 0){//
		//if (interrT1 != 0){
			freq = interrT0 / (2*interrT1);
			DisplayString(0,"# inter TMR0/sec");
			sprintf(freqString," %d ",(int) freq);
			DisplayString(16,freqString);
		}		
		TMR1F = 0;
    }
	
	if ( TMR0F ){
		interrT0++;
		TMR0F = 0;	
	}
}




void main(void)
    {
    int i;
    char freqString[32] = {0};
   
    // on configure les broches du pic comme étant des broches de sortie.
    // c'est à dire que les signaux extérieurs sont ignorés et que la tension
    // est fixée par le pic.
    LED0_TRIS = 0; //configure 1st led pin as output (yellow)
    LED1_TRIS = 0; //configure 2nd led pin as output (red)
    LED2_TRIS = 0; //configure 3rd led pin as output (red)
    
    LED0_IO = 0; //off 1st led (yellow)
    LED1_IO = 0; //off 2nd led (red)
    LED2_IO = 0; //off 3rd led (red)
  
    // Initialisation du lcd.
    LCDInit();
    for(i=0;i<100;i++) dumb_delay1ms();

    /* Configuration du timer */
	
	
	INTCONbits.GIE = 1; // enable interrupts
	
	RCONbits.IPEN        = 1;   //enable interrupts priority levels
	// Connecte les interruption TMR0 avec la priorité haute.
	INTCON2bits.TMR0IP	 = 1; 
	// Connecte les interruption TMR1 avec la priorité haute.
	IPR1bits.TMR1IP	 = 1;
	 
	// clear interruption flag	
	TMR0F    = 0;
    	
	// Configure le timer en 16 bits
	// il génère donc une interruption tous les 0xFFFF = 65536 tops 
	// d'horloge.
	T0CONbits.T08BIT    = 0;

	// Configure l'origine du signal du timer.
	// nous on veut qu'il compte les signaux de l'horloge interne.
	T0CONbits.PSA       = 1;

	// enable TMR0 interrupts
	INTCONbits.TMR0IE   = 1;
	// enable TMR1 interrupts
	PIE1bits.TMR1IE   = 1;
	
	// ca permet au timer de commencer à compter.
	T0CONbits.TMR0ON    = 1;
	T1CONbits.TMR1ON    = 1;
	
    
    // Boucle principale. en gros on rafraichit l'affichage 
    // chaque fois que le bit display est mis à 1.
    //  ce qui est fait dans les interruptions.
    while(1)
    {
		
	}
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
void DisplayWORD(BYTE pos, WORD w) //WORD is a 16 bits unsigned
{
    BYTE WDigit[6]; //enough for a  number < 65636: 5 digits + \0
    BYTE j;
    BYTE LCDPos=0;  //write on first line of LCD
    unsigned radix=10; //type expected by sdcc's ultoa()

    LCDPos=pos;
    ultoa(w, WDigit, radix);      
    for(j = 0; j < strlen((char*)WDigit); j++)
    {
       LCDText[LCDPos++] = WDigit[j];
    }
    if(LCDPos < 32u)
       LCDText[LCDPos] = 0;
    LCDUpdate();
}
/*************************************************
 Function DisplayString: 
 Writes an IP address to string to the LCD display
 starting at pos
*************************************************/
void DisplayString(BYTE pos, char* text)
{
   BYTE l= strlen(text)+1;
   BYTE max= 32-pos;
   strlcpy((char*)&LCDText[pos], text,(l<max)?l:max );
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
size_t
strlcpy(char *dst, const char *src, size_t siz)
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


void dumb_delay1ms (void)
{
    TMR0H = HIGH (0x10000-EXEC_FREQ/1000);
    TMR0L = LOW  (0x10000-EXEC_FREQ/1000);
    T0CONbits.TMR0ON = 0;  //disable timer0
    T0CONbits.T08BIT = 0;  //use timer0 16-bit counter
    T0CONbits.T0CS   = 0;  //use timer0 instruction cycle clock
    T0CONbits.PSA    = 1;  //disable timer0 prescaler
    INTCONbits.T0IF   = 0;  //clear timer0 overflow bit
    T0CONbits.TMR0ON = 1;  //enable timer0
    while(!INTCONbits.T0IF){} //busy wait for timer0 to overflow
    INTCONbits.T0IF   = 0;  //clear timer0 overflow bit
    T0CONbits.TMR0ON = 0;  //disable timer0   
}


