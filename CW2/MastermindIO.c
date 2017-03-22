#ifndef MASTERMINDIO
#define MASTERMINDIO


#include "LCDIO.c"
#include "GeneralIO.c"

static volatile int * gpio;
static LCD * screen;
static int redPin, greenPin;

initialiseMastermindIO(){
	int dataPins[4] = {23, 17, 27, 22};
	
	redPin = 5;
	greenPin = 6;
	gpio = getGPIO();
	
	screen = lcdFactory(2, 16, lcdPinSetFactory(gpio, 24, 25, dataPins));
	
	lcdInitialise(gpio, screen);
	
	pinMode(gpio, redPin, GPFSEL_OUTPUT);
	digitalWrite(gpio, redPin, 0);
	
	pinMode(gpio, greenPin, GPFSEL_OUTPUT);
	digitalWrite(gpio, greenPin, 0);
}

lcdShowResult(int exact, int approximate){
	char string[20];
	
	lcdClear(gpio, screen);

	
	sprintf(string, "Exact: %d", exact);	
	lcdPutString(gpio, screen, string, 0, 0);
	
	lcdNewLine(gpio, screen);
	
	sprintf(string, "Approximate: %d", approximate);
	lcdPutString(gpio, screen, string, 0, 0);
}

lcdSuccess(int attempts){
	char string[20];
	
	lcdClear(gpio, screen);
	lcdPutString(gpio, screen, "Congratulations!", 0, 350000);
	lcdNewLine(gpio, screen);
	
	sprintf(string, "Attempts: %d", attempts);
	lcdPutString(gpio, screen, string, 0, 0);
}

pinFlash(int pin, int times){
	int i =0;
	
	for (; i<times; i++){
		digitalWrite(gpio, pin, 1);
		usleep(500000);
		digitalWrite(gpio, pin, 0);
		usleep(500000);
	}	
}

redFlash(int times){
	pinFlash(redPin, times);
}

greenFlash(int times){
	pinFlash(greenPin, times);
}

ledShowResult(int exact, int approximate){
	
	greenFlash(exact);
	redFlash(1);
	greenFlash(approximate);
	redFlash(3); //New round signifier
}

ledSuccess(){
	digitalWrite(gpio, redPin, 1);
	greenFlash(3);
	digitalWrite(gpio, redPin, 0);
}

#endif
