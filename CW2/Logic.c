#include <stdio.h>
#include <stdlib.h> //contains rand
#include <unistd.h> //Used for geopt
#include <time.h> //Used to seed rand
#include <string.h> // memcpy

#include "MastermindIO.c"

struct {
	int exact;
	int approximate;
} typedef Result;


int debug, codeLength;

Result checkGuess(int * guess, int * answer){
	int index =0, approx=0, exact=0, inner;
	Result res= {0, 0};

	//Since this function alters the guess and answer arrays, a copy is required.
	int * answercpy = malloc(sizeof(*answercpy) * codeLength);
	memcpy(answercpy, answer, sizeof(*answercpy) * codeLength);

	//Note that all exact checks must be performed before
	//inexact checking. This is beacuse some approximate gusses
	//may end up overwriting exact matches otherwise.

	for (;index<codeLength;index++){
		if (guess[index] == answercpy[index]){
			exact++;
			guess[index]= -1;
			answercpy[index]=-1;
		}
	}
	for (index=0;index<codeLength;index++){
		for (inner=0;inner<codeLength; inner++){
			if (inner!= index && guess[index] == answercpy[inner] && guess[index] != -1){
				approx++;
				guess[index]=-1;
				answercpy[inner]=-1;
			}
		}
	}

	free(answercpy);
	free(guess);

	//This function cannot return guess, as it has modified the values.
	res.exact=exact;
	res.approximate = approx;
	return res;
}

int * getGuess(){
	int * guess;
	int i;

	guess = malloc(sizeof(*guess) * codeLength);
	for (i = 0; i<codeLength; i++){
		guess[i] = getButtonInput();
		if (debug) printf("Input: %d\n", guess[i]);
		ledInputRecieved(guess[i]);
	}

	redFlash(2);

	return guess;
}

void showResult(Result res){

	lcdShowResult(res.exact, res.approximate);
	ledShowResult(res .exact, res.approximate);

	if(debug) printf("Exact: %d\nApproximate: %d\n", res.exact, res.approximate);
}

int * generateAnswer(int colourCount){
	int * answer;
	answer = malloc(sizeof(*answer) * codeLength);
	int i;

	if (debug) printf("Secret code: ");

	for (i=0; i<codeLength; i++){
		answer[i] = (rand() % colourCount) +1; //+1 required else answer code may include 0
		if (debug) printf("%d  ", answer[i]);
	}

	if (debug) printf("\n");

	return answer;
}

void main(int argc, char ** argv){
	int * answer, * guess;
	Result res;
	int opt, cCount, attempts;

	debug = 0; //Initialise
	codeLength = 3;
	cCount=3;
	attempts=0;
	srand(time(NULL)); //NULL is where time() would otherwise store the result of the call

	answer=malloc(sizeof(*answer) * codeLength);

	initialiseMastermindIO();

	

	opterr=0;
	while((opt=getopt(argc, argv, "dc:n:")) != -1){
		switch(opt){
		case 'd':
			debug = 1;
			break;
		case 'c':
			cCount = (*optarg -48); //optarg is a char, and some conversion is required
			break;
		case 'n':
			codeLength = (*optarg - 48); //see above
			break;
		case'?':
			switch (optopt){
			case 'n' :
				printf("Missing required option argument: \n-n \e[4mcode_length\e[0m = Specify the length of code to guess\n");
				break;
			case 'c':
				printf("Missing required option argument: \n-c \e[4mno_of_colours\e[0m = Specify the number of colours used\n");
				break;
			default:
				printf("Unidentified option: %c\n", optopt);
				break;
			}
			exit(0);
		}
	}

	if (cCount<=0 || codeLength<=0) {
		printf("Argument options must be greater than 0");
		exit(0);
	}

	answer = generateAnswer(cCount);
	welcomeMessage();

	do {
		lcdInputPrompt();		//begin thread
		guess = getGuess();
		lcdInputReceived();		//end thread
		attempts++;
		res = checkGuess(guess, answer);
		if (res.exact == codeLength) break;
		showResult(res);
		usleep(100000);
	} while (1);

	lcdSuccess(attempts);
	ledSuccess();
}
