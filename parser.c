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

typedef struct {
	char * comment;
	struct Node * next;
} Node;

typedef struct {
	int commentcount;
	Node * commentlist;
	int width;
	int height;
	int maxval;
	Pixel * pixellist;

} PPM;
/**
 * A preliminary function which given a filepath will parse and validate a PPM file.
 */
PPM parsefile(char * file) {
	PPM ppmfile;
	Node * listindex;
	FILE * fin;
	char linein[1000];
	regex_t regex;
	int int1, int2, int3, err, pixelcount;

	fin = fopen(file, "r");
	if (fin == NULL) {
		printf("Unable to open file: %s", file);
	}

	fgets(linein, 999, fin);
	if (applyregex("P3", linein)) { //Checks first line of file is "P3"
	/*throw new*/formatexception();
	}

	listindex = (Node *) malloc(sizeof(Node));
	ppmfile.commentlist = listindex; //Both point to the empty list node

	if (fgets(linein, 999, fin) == NULL) { //If EOF is hit (or an error)
			formatexception();
		}
	
	while (applyregex("#.*", linein) == 0) {	//Read lines until the first non comment line is reached

		listindex->comment = linein;
		listindex->next = (Node *) malloc(sizeof(Node));
		printf("%s", listindex->comment);
		listindex = listindex->next;
		ppmfile.commentcount = ppmfile.commentcount + 1;
		if (fgets(linein, 999, fin) == NULL) { 	//If EOF is hit (or an error)
			formatexception();
		}
		
	} 

	if (sscanf(linein, "%d %d", &ppmfile.width, &ppmfile.height) != 2) { //The Comment regex loop reads the first line after the comments into linein, hence scan on string.
		formatexception(); //They are the right way round; width then height.
	}

	if (ppmfile.width <= 0 || ppmfile.height <= 0) {
		formatexception();
	}

	printf("%d\t", ppmfile.width);
	printf("%d\n", ppmfile.height);

	ppmfile.pixellist = (Pixel *)malloc((ppmfile.width*ppmfile.height)*sizeof(Pixel));

	fgets(linein, 999, fin);
	sscanf(linein, "%d", &ppmfile.maxval); //reads in max value. Should discard other characters on the same line
	if (ppmfile.maxval <= 0) {
		formatexception();
	}

	printf("%d\n", ppmfile.maxval);

	err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Read in the first three RGB values; 1 pixel.
	pixelcount = 0; //Barring error (where it doesn't matter), is set to 1 immediately in the loop.
	while (err == 3) { //While 3 values have successfully been read.
		pixelcount++;


		if (int1 > ppmfile.maxval || int2 > ppmfile.maxval || int3 > ppmfile.maxval || pixelcount > (ppmfile.width * ppmfile.height)) { //Check for validity
			formatexception();
		}
		Pixel temp = {int1, int2, int3};
		ppmfile.pixellist[pixelcount] =temp;
		printf("%d %d %d\n",ppmfile.pixellist[pixelcount].red ,ppmfile.pixellist[pixelcount].blue, ppmfile.pixellist[pixelcount].green);
		err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Try to read next three values

	}

	if (err == EOF) {
		printf("End of File\n");
	} else {
		//Pixels not multiple of three, or potentially another error.
		formatexception();
	}

	if (pixelcount != (ppmfile.width * ppmfile.height)) {
		formatexception();
	}

	return ppmfile;
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
		exit(0);
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
	exit(0);
}


main(int argc, char ** argv) {

parsefile(argv[1]);


}
