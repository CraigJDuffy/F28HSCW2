#ifndef LCDIO_H
#define LCDIO_H

#include <time.h>
#include <stdlib.h>

#include "GeneralIO.c"

//Command bit sequences (all flags 0) for Hitachi HD44780U
#define	INST_CLEAR	0x01
#define	INST_HOME	0x02
#define	INST_ENTRY	0x04
#define	INST_DISPLAY	0x08
#define	INST_SHIFT	0x10
#define	INST_FUNCSET	0x20
#define	INST_CGADDR	0x40
#define	INST_DDADDR	0x80

//Flag bits for above instructions
#define	ENTRY_SHIFT	0x01
#define	ENTRY_INC	0x02

#define DISPLAY_BLINK 0x01
#define	DISPLAY_CURSOR	0x02
#define	DISPLAY_DISPLAY	0x04

#define SHIFT_RIGHT 0x04
#define SHIFT_DISPLAY 0x08

#define	FUNCSET_FONT	0x04
#define	FUNCSET_ROWS	0x08
#define	FUNCSET_DATALEN	0x10

struct {
	int strobe, registerSelect;
	int data[4];
} typedef LCDPinSet;

struct {
	LCDPinSet * pins;
	int rows, columns, cursorX, cursorY;
} typedef LCD;


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// As a rule of thumb, those functions which take LCDPinSet * pins
// as an argument should not be directly accessed. Not unless you
// undertstand the underlyng code structure and behaviour.
//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

lcdStrobe(volatile int * gpio, LCDPinSet * pins){
	digitalWrite(gpio, pins->strobe, 1);
	usleep(200);
	digitalWrite(gpio, pins->strobe, 0);
	usleep(200);
}

lcd4BitData(volatile int * gpio, LCDPinSet * pins, unsigned char data){
	int i =0;
	for (;i<4;i++){
		digitalWrite(gpio, pins->data[i], (data&1));
		data>>=1;
	}
	
	lcdStrobe(gpio, pins);
	
}

lcd8BitData(volatile int * gpio, LCDPinSet * pins, unsigned char data){
	//Note, send high four bits first.
	lcd4BitData(gpio, pins, ((data>>4) & 0x0F));
	lcd4BitData(gpio, pins, (data & 0x0F));
}

lcdCommand(volatile int * gpio, LCD * screen, unsigned char command){
	digitalWrite(gpio, screen->pins->registerSelect, 0);
	lcd8BitData(gpio, screen->pins, command);
}

lcdCarriageReturn(volatile int * gpio, LCD * screen){
	screen->cursorX = 0;
	lcdCommand(gpio, screen, screen->cursorX + (INST_DDADDR | (screen->cursorY==1 ? 0x40 : 0x00)));
}

lcdLineFeed(volatile int * gpio, LCD * screen){
	screen->cursorY++;
	if(screen->cursorY >= screen->rows){
		screen->cursorY = 0;
	}
	
	lcdCommand(gpio, screen, screen->cursorX + (INST_DDADDR | (screen->cursorY==1 ? 0x40 : 0x00)));
}

lcdNewLine(volatile int * gpio, LCD * screen){
	lcdLineFeed(gpio, screen);
	lcdCarriageReturn(gpio, screen);
}

lcdClear(volatile int * gpio, LCD * screen){
	lcdCommand(gpio, screen, INST_CLEAR);
	usleep(50000);
	screen->cursorX =0;
	screen->cursorY =0;
}

lcdWrite(volatile int * gpio, LCD * screen, unsigned char data){
	digitalWrite(gpio, screen->pins->registerSelect, 1);
	lcd8BitData(gpio, screen->pins, data);
	screen->cursorX++;
}

lcdPutString(volatile int * gpio, LCD * screen, char * string, int lineWrapping, int udelay){
	while (*string){
		lcdWrite(gpio, screen, *string++);
		if (lineWrapping && screen->cursorX == screen->columns)
			lcdNewLine(gpio, screen);
		
		usleep(udelay);
	}
}

lcdFauxLoading(volatile int * gpio, LCD *  screen, int utime){
	
	do{
	lcdCommand(gpio, screen, INST_CLEAR);
	usleep(10000);
	lcdPutString(gpio, screen, "Loading...", 0, 200000);
	utime -= 2000000;
	} while (utime>0);
	
}

lcdFauxBusy(volatile int * gpio, LCD * screen, int utime){
	unsigned char characters[4] = {0b01111100, 0b00101111, 0b00101101, 0b00000000};
		int i =0;
	
	do {
		for (i=0; i<4; i++){
			lcdWrite(gpio, screen, characters[i]);
			usleep(250000);
			lcdCommand(gpio, screen, --screen->cursorX + (INST_DDADDR | (screen->cursorY==1 ? 0x40 : 0x00)));
		}
		utime -= 1000000;
	} while (utime >0);
		
}

lcdDefineBackslash(volatile int * gpio, LCD * screen){
	
	lcdCommand(gpio, screen, INST_CGADDR | 0b0000000000);
	
	lcdWrite(gpio, screen, 0b00000000);
	
	lcdWrite(gpio, screen, 0b00010000);

	lcdWrite(gpio, screen, 0b00001000);
	
	lcdWrite(gpio, screen, 0b00000100);
	
	lcdWrite(gpio, screen, 0b00000010);
	
	lcdWrite(gpio, screen, 0b00000001);
	
	int i =0;
	
	for (;i<10; i++){
		lcdWrite(gpio, screen, 0b00000000);
	}
	
	lcdCommand(gpio, screen, INST_DDADDR);
	lcdCommand(gpio, screen, INST_CLEAR);
	lcdCommand(gpio, screen, INST_HOME);
	
	screen->cursorX = 0;
	screen->cursorY = 0;
}


lcdInitialise(volatile int * gpio, LCD * screen){
	usleep(50000);
	lcd4BitData(gpio, screen->pins, 3);
	usleep(10000);
	lcd4BitData(gpio, screen->pins, 3);
	usleep(10000);
	lcd4BitData(gpio, screen->pins, 3);
	usleep(10000);
	lcd4BitData(gpio, screen->pins, 2); //4-bit mode here
	
	
	lcdCommand(gpio, screen, INST_FUNCSET | FUNCSET_ROWS );
	lcdCommand(gpio, screen, INST_DISPLAY);
	lcdCommand(gpio, screen, INST_CLEAR);
	lcdCommand(gpio, screen, INST_ENTRY | ENTRY_INC);
	usleep(10000);
	
	lcdDefineBackslash(gpio, screen);
	
	lcdCommand(gpio, screen, INST_DISPLAY | DISPLAY_DISPLAY);
	
	usleep(10000);
}

LCDPinSet * lcdPinSetFactory(volatile int * gpio, int strobe, int regselect, int * data){
	LCDPinSet * pins = malloc(sizeof(LCDPinSet));
	if (pins == NULL){
		printf("Malloc failure");
		exit(0);
	}
	
	digitalWrite(gpio, strobe, 0);
	pinMode(gpio, strobe, GPFSEL_OUTPUT);
	pins->strobe = strobe;
	
	digitalWrite(gpio, regselect, 0);
	pinMode(gpio, regselect, GPFSEL_OUTPUT);
	pins->registerSelect = regselect;
	
	int i = 0;
	
	for (; i<4; i++){
		digitalWrite(gpio, data[i], 0);
		pinMode(gpio, data[i], GPFSEL_OUTPUT);
		pins->data[i] = data[i];
	}
	
	return pins;
	
}

LCD * lcdFactory(int rows, int columns, LCDPinSet * pins){
	
	if (rows>2){
		printf("Max supported: 2 rows");
		exit(0);
	}
	
	LCD * screen = malloc(sizeof(LCD));
	if (screen == NULL) {
		printf("Malloc failure");
		exit(0);
	}
		
	screen->rows = rows;
	screen->columns = columns;
	screen->cursorX = 0;
	screen->cursorY = 0;
	screen->pins = pins;
	
	return screen;
} 

#endif
