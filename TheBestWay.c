/*
 * TheBestWay.c
 *
 *  Created on: 28 Jan 2017
 *      Author: Tommy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>

typedef struct {
	int red;
	int green;
	int blue;
} Pixel;


int encodeflag = 0, decodeflag = 0, verboseflag=0;

/**
 * Encodes a given integer into a Pixel format.
 * The integer is encoded as an absolute value for the red value, and as an offset for the green value.
 * The encoding provides for (maxval+1)*(maxval) possible values.
 */
Pixel encodechar(Pixel original, int character, int maxval) {
	int rcode, gdiff;
	int c = character;

	rcode = c % (maxval + 1);
	gdiff = c / (maxval + 1); //Truncated towards 0. If maxval is negative the effect is ceiling, not floor.

	if (gdiff > maxval) {
		printf("Unable to encode character %c with max colour value of %d", character, maxval);
		exit(0);
	}

	Pixel encoded = {rcode, ((original.green + gdiff) % (maxval + 1)), original.blue};

	return encoded;

}

/**
 * Decodes the integer value from a given Pixel compared to the original.
 * Function will always return a value, even if the pixels are identical.
 * Behaviour where max value is inconsistent with the encoding is undefined.
 */
int decodepixel(Pixel original, Pixel encoded, int maxval) {
	int factor;

	if (encoded.green >= original.green) {
		factor = encoded.green - original.green;
	} else {
		factor = ((maxval + 1) - original.green) + encoded.green;
	}

	return encoded.red + (factor * (maxval + 1));

}

/**
 * A preliminary function which given a filepath will parse and validate a PPM file.
 */
encode(char * message, FILE * fin) {
	char linein[1000];
	char * c;
	regex_t regex;
	int width, height, maxval, err, pixelcount, randmax, randdiff;
	Pixel pix;

	if (fin == NULL) {
		printf("Unable to open PPM file");
		exit(0);
	}

	fgets(linein, 999, fin);
	if (applyregex("P3", linein)) { //Checks first line of file is "P3"
	/*throw new*/formatexception();
	}

	printf("%s", linein);

	do {
		if (fgets(linein, 999, fin) == NULL) { //If EOF is hit (or an error)
			formatexception();
		}
		printf("%s", linein);
	} while (applyregex("#.*", linein) == 0); //Read lines until the first non comment line is reached

	//Note that the Height and Width are printed out within the while loop.

	if (sscanf(linein, "%d %d", &width, &height) != 2) { //The Comment regex loop reads the first line after the comments into linein, hence scan on string.
		formatexception(); //They are the right way round; width then height.
	}

	if (width <= 0 || height <= 0) {
		formatexception();
	}

	fgets(linein, 999, fin);
	sscanf(linein, "%d", &maxval); //reads in max value. Should discard other characters on the same line
	if (maxval <= 0) {
		formatexception();
	}

	printf("%d\n", maxval);


	srand(width*height);
	randmax = (height*width)/(strlen(message));//randmax is the maximum of rand such that the message will fit.
	if(randmax=0 || ((maxval+1) * maxval)<256){//If too few pixels or too little colour values to encode ASCII
		printf("The message is too long for the format of the PPM file");
		exit(0);
	}
	randdiff=rand()%randmax; //randdiff is an offset from the current position to the next pixel to encode.

	err = fscanf(fin, "%d %d %d", &pix.red, &pix.green, &pix.blue); //Read in the first three RGB values; 1 pixel.
	pixelcount = 0; //Barring error (where it doesn't matter), is set to 1 immediately in the loop.
	while (err == 3) { //While 3 values have successfully been read.
		pixelcount++;

		if (pix.green > maxval || pix.red > maxval || pix.blue > maxval || pixelcount > (width * height)) { //Check for validity
			formatexception();
		}

		if (randdiff=0){
			randdiff = rand()%randmax;
			sscanf(message, "%c", c);
			pix = encodechar(pix, c, maxval);
		}
		printf("%d %d %d\n", pix.red, pix.green, pix.blue);
		err = fscanf(fin, "%d %d %d", &pix.red, &pix.green, &pix.blue); //Try to read next three values

	}

	if (err == EOF) {
		printf("End of File\n");
	} else {
		//Pixels not multiple of three, or potentially another error.
		formatexception();
	}

	if (pixelcount != (width * height)) {
		formatexception();
	}
}


/**
 * Function applies a given regex to a string, returning 0 on match or 1 on fail.
 * Function may return other values in the event of internal error. These can be identified using the
 * regerror function in the regex library.
 */
int applyregex(char* regex, char* string) {
	int result;
	regex_t expression;

	if (regcomp(&expression, regex, 0)) { //compile the regex string into address expression with flags of 0. Return 0 on success.
		printf("Error compiling regex.\n");
		exit(0);
	}

	result = regexec(&expression, string, 0, NULL, 0); //Execute compiled regex at address expression, apply to string, with flags: 0, null, 0.
	regfree(&expression); //Release memory associated with the compiled regex as it's no longer needed.
	return result;
}

/*
 * Function reports a file format exception before exiting.
 */
formatexception() {
	printf("\nFile format exception\n");
	exit(0);
}

main(int argc, char ** argv) {
	char * decodefile, message[200];
	int opt;

	while((opt = getopt(argc, argv, "ed:")) != -1)//Cycle through all flags; valid options are -e and -d with argument
		switch (opt){
		case 'e':
			encodeflag=1;
			break;
		case 'd':
			decodefile = optarg;
			decodeflag=1;
			break;
		}



	if(encodeflag){
		printf("Input the message\n>");
		fgets(message, 200, stdin);
		encode(message, fopen(argv[optind],"r" ));
	} else if(decodeflag){
		//decode(fopen(argv[optind], "r"), fopen(decodefile, "r"));
	}
}
