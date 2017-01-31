/*
 * parser.c
 *
 *  Created on: 28 Jan 2017
 *      Author: Tommy
 */

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

typedef struct {
	int red;
	int green;
	int blue;
} Pixel;


/**
 * A preliminary function which given a filepath will parse and validate a PPM file.
 */
parsefile(char * file) {
	FILE * fin;
	char linein[1000];
	regex_t regex;
	int width, height, maxval, int1, int2, int3, err, pixelcount;

	fin = fopen(file, "r");

	if (fin == NULL) {
		printf("Unable to open file: %s", file);
	}

	fgets(linein, 999, fin);
	if (applyregex("P3", linein)) { //Checks first line of file is "P3"
	/*throw new*/formatexception();
	}

	do {
		if (fgets(linein, 999, fin) == NULL) { //If EOF is hit (or an error)
			formatexception();
		}
		//Put comment lines into Struct here
	} while (applyregex("#.*", linein) == 0); //Read lines until the first non comment line is reached

	if (sscanf(linein, "%d %d", &width, &height) != 2) { //The Comment regex loop reads the first line after the comments into linein, hence scan on string.
		formatexception(); //They are the right way round; width then height.
	}

	if (width <= 0 || height <= 0) {
		formatexception();
	}

	printf("%d\t", width);
	printf("%d\n", height);

	fgets(linein, 999, fin);
	sscanf(linein, "%d", &maxval); //reads in max value. Should discard other characters on the same line
	if (maxval <= 0) {
		formatexception();
	}

	printf("%d\n", maxval);

	err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Read in the first three RGB values; 1 pixel.
	pixelcount = 0; //Barring error (where it doesn't matter), is set to 1 immediately in the loop.
	while (err == 3) { //While 3 values have successfully been read.
		pixelcount++;


		if (int1 > maxval || int2 > maxval || int3 > maxval || pixelcount > (width * height)) { //Check for validity
			formatexception();
		}

		printf("%d %d %d\n", int1, int2, int3);
		err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Try to read next three values

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
		memexit();
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
 * Function applies a given regex to a string, returning 0 on match or 1 on fail.
 * Function may return other values in the event of internal error. These can be identified using the
 * regerror function in the regex library.
 */
int applyregex(char* regex, char* string) {
	int result;
	regex_t expression;

	if (regcomp(&expression, regex, 0)) { //compile the regex string into address expression with flags of 0. Return 0 on success.
		printf("Error compiling regex.\n");
		regfree(&expression);//free memory that may or may not have been allocated. May or may not throw an error of it's own.
		memexit();
	}

	result = regexec(&expression, string, 0, NULL, 0); //Execute compiled regex at address expression, apply to string, with flags: 0, null, 0.
	regfree(&expression); //Release memory associated with the compiled regex as it's no longer needed.
	return result;
}

/**
 * Function reports a file format exception before exiting.
 */
formatexception() {
	printf("\nFile format exception\n");
	memexit();
}

/**
 * A function to deallocate all memory before exiting.
 *
 */
memexit(){
	//free memory
	exit(0);
}

main(int argc, char ** argv) {

parsefile(argv[1]);


}
