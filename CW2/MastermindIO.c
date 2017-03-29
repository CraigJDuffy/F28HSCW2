#ifndef MASTERMINDIO
#define MASTERMINDIO

#include <pthread.h>
#include "LCDIO.c"
#include "GeneralIO.c"



static volatile int * gpio;
static LCD * screen;
static int redPin, greenPin;
pthread_t threadID;





//
//LCD functions
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


/*
 * Function to show the results of a guess on the LCD
 * */
lcdShowResult(int exact, int approximate){
	char string[20];

	lcdClear(gpio, screen);


	sprintf(string, "Exact: %d", exact);
	lcdPutString(gpio, screen, string, 0, 0);

	lcdNewLine(gpio, screen);

	sprintf(string, "Approximate: %d", approximate);
	lcdPutString(gpio, screen, string, 0, 0);

}

/*Function to display appropriate success message on LCD
 * */
lcdSuccess(int attempts){
	char string[20];

	lcdClear(gpio, screen);
	lcdPutString(gpio, screen, "Congratulations!", 0, 350000);
	lcdNewLine(gpio, screen);

	sprintf(string, "Attempts: %d", attempts);
	lcdPutString(gpio, screen, string, 0, 0);
}

/*
 * A convenience wrapper function for the lcdBusy function to ease thread creation
 * */
void * lcdBusyThread(void * arg){
	lcdBusy(gpio, screen);
}

/*
 * A function which prompts the user for input via the LCD.
 * Starts a thread to display the busy symbol. NOTE that no other functions
 * should access the LCD after invoking this function without first invoking
 * lcdInputReceived.
 * */
lcdInputPrompt(){
	lcdClear(gpio, screen);
	lcdHome(gpio, screen);
	lcdPutString(gpio, screen, "Awaiting Input", 0, 0);
	if (threadID == -314){
		pthread_create(&threadID, NULL, lcdBusyThread, NULL);
	}
}

/*
 * Function to kill the thread showing the busy symbol after input has been recieved.
 * This allows other functions to access the LCD.
 * */
lcdInputReceived(){
	if (threadID != -314){
		pthread_cancel(threadID);
		threadID = -314;
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




//
//LED Functions
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/* Lower level function for flashing a given LED a number of times
 * */
pinFlash(int pin, int times){
	int i =0;

	for (; i<times; i++){
		digitalWrite(gpio, pin, 1);
		usleep(500000);
		digitalWrite(gpio, pin, 0);
		usleep(500000);
	}
}

/*
 * simple function to flash the red LED a number of times
 * */
redFlash(int times){
	pinFlash(redPin, times);
}

/*
 * Simple function to flash the green led a number of times.
 * */
greenFlash(int times){
	pinFlash(greenPin, times);
}

/*
 * Function for showing the results via LEDs
 * */
ledShowResult(int exact, int approximate){

	greenFlash(exact);
	redFlash(1);
	greenFlash(approximate);
	redFlash(3); //New round signifier
}

/*
 * Function to display the end of the game on LEDs
 * */
ledSuccess(){
	digitalWrite(gpio, redPin, 1);
	greenFlash(3);
	digitalWrite(gpio, redPin, 0);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




//
//Speciality functions
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


/*
 * Function which sets up all the IO for the Mastermind game
 * as per the coursework spec.
 * */
initialiseMastermindIO(){
	int dataPins[4] = {23, 17, 27, 22};

	redPin = 5;
	greenPin = 6;
	threadID = -314;
	gpio = getGPIO();

	screen = lcdFactory(2, 16, lcdPinSetFactory(gpio, 24, 25, dataPins));

	lcdInitialise(gpio, screen);

	pinMode(gpio, redPin, GPFSEL_OUTPUT);
	digitalWrite(gpio, redPin, 0);

	pinMode(gpio, greenPin, GPFSEL_OUTPUT);
	digitalWrite(gpio, greenPin, 0);
}

/*
 * A large, highly bespoke function used to display the intro sequence.
 * This function could benefit from some refactoring, with some special
 * effects being abstracted into more general functions within the LCDIO
 * source code. That might bloat the LCD library however.
 * */
welcomeMessage(){
	digitalWrite(gpio, redPin, 1);
	digitalWrite(gpio, greenPin, 1);

	lcdPutString(gpio, screen, "Duffy ", 0, 0);
	lcdLineFeed(gpio, screen);
	lcdPutString(gpio, screen, "and ", 0, 0);
	lcdLineFeed(gpio, screen);
	lcdPutString(gpio, screen, "Lamb", 0, 0);
	lcdNewLine(gpio, screen);
	usleep(500000);
	lcdPutString(gpio, screen, "Proudly  Present", 0, 250000);
	usleep(2000000);


	lcdHome(gpio, screen);
	lcdClear(gpio, screen);
	lcdPutString(gpio, screen, "Proudly  Present", 0, 0);
	usleep(1000000);
	lcdHome(gpio, screen);
	lcdClear(gpio, screen);
	lcdNewLine(gpio, screen);
	lcdPutString(gpio, screen, "A Hans-Wolfgang", 0, 0);
	usleep(1000000);
	lcdClear(gpio, screen);
	lcdHome(gpio, screen);
	lcdPutString(gpio, screen, "A Hans-Wolfgang", 0, 0);
	lcdNewLine(gpio, screen);
	lcdPutString(gpio, screen, "Production...", 0, 0);
	usleep(1000000);


	screen->cursorX=21;
	screen->cursorY=0;
	lcdSyncCursor(gpio, screen);

	lcdPutString(gpio, screen, "Mastermind", 0, 0);

	int i =0;
	for (; i<21;i++){
		usleep(250000);
		lcdCommand(gpio, screen, INST_SHIFT | SHIFT_DISPLAY);
	}

	screen->cursorX=34;
	screen->cursorY=1;
	lcdSyncCursor(gpio, screen);
	lcdCommand(gpio, screen, INST_ENTRY);
	lcdCommand(gpio, screen, INST_DISPLAY | DISPLAY_DISPLAY | DISPLAY_CURSOR | DISPLAY_BLINK);

	lcdPutString(gpio, screen, "noitidE iP", 0, 125000);

	usleep(3000000);

	lcdCommand(gpio, screen, INST_ENTRY | ENTRY_INC);
	lcdCommand(gpio, screen, INST_DISPLAY | DISPLAY_DISPLAY);
	lcdClear(gpio, screen);
	lcdHome(gpio, screen);

	digitalWrite(gpio, redPin, 0);
	digitalWrite(gpio, greenPin, 0);

}
#endif
