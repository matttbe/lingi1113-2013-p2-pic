AS = gpasm
CC = sdcc
CFLAGS= -c -mpic16 -p18f97j60 -V -o$@ 
LD = sdcc

ifndef PREFIX
	PREFIX=/usr/local
endif

LDFLAGS= -mpic16 -p18f97j60 -L$(PREFIX)/lib/pic16 -L$(PREFIX)/share/sdcc/lib/src/pic16/startup/ -L$(PREFIX)/share/sdcc/lib/src/pic16/libc/ -llibio18f97j60.lib \
         -llibdev18f97j60.lib -llibc18f.a
AR = ar
RM = rm

OBJECTS= Objects/LCDBlocking.o

SDCC_HEADERS=$(PREFIX)/share/sdcc/include/string.h \
   $(PREFIX)/share/sdcc/include/stdlib.h \
   $(PREFIX)/share/sdcc/include/stdio.h \
   $(PREFIX)/share/sdcc/include/stddef.h \
   $(PREFIX)/share/sdcc/include/stdarg.h 

SDCC_PIC16_HEADERS=$(PREFIX)/share/sdcc/include/pic16/pic18fregs.h

TCPIP_HEADERS=   Include/TCPIP_Stack/ETH97J60.h \
   Include/TCPIP_Stack/LCDBlocking.h 

APP_HEADERS=Include/GenericTypeDefs.h \
   Include/Compiler.h \
   Include/HardwareProfile.h 

clock : Objects/clock.o $(OBJECTS)
	$(LD) $(LDFLAGS) Objects/clock.o $(OBJECTS)

led : Objects/led.o
	$(LD) $(LDFLAGS) Objects/led.o

Objects/clock.o : clock.c $(SDCC_HEADERS) $(SDCC_PIC16_HEADERS) \
   $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) $(CFLAGS) clock.c

Objects/led.o : led.c $(SDCC_HEADERS) $(SDCC_PIC16_HEADERS) \
   $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) $(CFLAGS) led.c


Objects/LCDBlocking.o : TCPIP_Stack/LCDBlocking.c $(SDCC_HEADERS)  \
   $(SDCC_PIC16_HEADERS) $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) -c -mpic16 -p18f97j60  -o"Objects/LCDBlocking.o" \
              -L$(PREFIX)/lib/pic16  TCPIP_Stack/LCDBlocking.c

Objects/Tick.o : TCPIP_Stack/Tick.c  $(SDCC_HEADERS)  \
   $(SDCC_PIC16_HEADERS) $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) -c -mpic16 -p18f97j60  -o"Objects/Tick.o" \
              -L$(PREFIX)/lib/pic16  TCPIP_Stack/Tick.c

clean : 
	$(RM) $(OBJECTS)


