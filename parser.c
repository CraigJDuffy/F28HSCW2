/*
 * parser.c
 *
 *  Created on: 28 Jan 2017
 *      Author: Tommy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //For strlen and strcpy
#include <unistd.h> //Only for usleep
#include <regex.h>

typedef struct {
	int red;
	int green;
	int blue;
	int dirty;
} Pixel;

typedef struct {
	char * comment;
	struct Node * next;
} Node;

typedef struct {
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

	ppmfile.commentlist = (Node *) malloc(sizeof(Node));
	listindex = ppmfile.commentlist; //Both point to the empty list node

	if (fgets(linein, 999, fin) == NULL) { //If EOF is hit (or an error)
			formatexception();
		}
	
	while (applyregex("#.*", linein) == 0) {	//Read lines until the first non comment line is reached

		listindex->comment = (char *) malloc(strlen(linein));//Space must be allocated for the next line to work, otherwise malloc memory gets corrupted.
																//Probably from trying to copy the string into unallocated space
		strcpy(listindex->comment,linein);	//Copies the string (char *) linein into the comment string(char *).Straight up comment=linein doesn't work, as the address of linein doesn't ever change but the referenced string does.
											//In other words all copies of comment will point to linein, whose value changes, such that all comments become the last thing stored in linein.
		listindex->next = (Node *) malloc(sizeof(Node));
		listindex = listindex->next;
		listindex->next = NULL;
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
	pixelcount = 0;
	while (err == 3) { //While 3 values have successfully been read.
		if (int1 > ppmfile.maxval || int2 > ppmfile.maxval || int3 > ppmfile.maxval || pixelcount >= (ppmfile.width * ppmfile.height)) { //Check for validity
			formatexception();
		}
		Pixel temp = {int1, int2, int3, 0};
		ppmfile.pixellist[pixelcount] =temp;
		printf("%d %d %d\n",ppmfile.pixellist[pixelcount].red ,ppmfile.pixellist[pixelcount].green, ppmfile.pixellist[pixelcount].blue);
		pixelcount++;
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
 * Given a PPM image the function prints the corresponding file format to standard out.
 */
showPPM(PPM image){
	Node * listindex;
	int pixelcount=0;
	
	printf("P3\n");
	
	listindex=image.commentlist;
	while(listindex->next != NULL){
		printf("%s", listindex->comment);
		listindex = listindex->next;
	}
	
	printf("%d %d\n", image.width, image.height);
	printf("%d\n", image.maxval);
	
	for(pixelcount=0;pixelcount<image.width*image.height;pixelcount++)
		printf("%d %d %d\n", image.pixellist[pixelcount].red, image.pixellist[pixelcount].green, image.pixellist[pixelcount].blue);
}



/**
 * Encodes a given integer into a Pixel format.
 * The integer is encoded as an absolute value for the red value, and as an offset for the green value.
 * The encoding provides for (maxval+1)*(maxval) possible values.
 * The function does not check the cleanliness of the give pixel.
 */
Pixel encodechar(Pixel original, unsigned int character, int maxval) {
	int rcode, gdiff;
	unsigned int c = character;

	rcode = c % (maxval + 1);
	gdiff = c / (maxval + 1); //Truncated towards 0. If maxval is negative the effect is ceiling, not floor.

	if ((gdiff) > maxval) {
		printf("Unable to encode character %c with max colour value of %d\n", character, maxval);
		exit(0);
	}

	Pixel encoded = {rcode, ((original.green + gdiff) % (maxval + 1)), original.blue +1, 1}; //1 is added to the blue field so that dirty pixels can always be identified when compared with the original

	return encoded;

}

/**
 * Decodes the integer value from a given Pixel compared to the original.
 * Function will always return a value, even if the pixels are identical.
 * Behaviour where max value is inconsistent with the encoding is undefined.
 */
unsigned int decodepixel(Pixel original, Pixel encoded, int maxval) {
	int factor;

	if (encoded.green >= original.green) {
		factor = encoded.green - original.green;
	} else {
		factor = ((maxval + 1) - original.green) + encoded.green;
	}

	return encoded.red + (factor * (maxval + 1));

}


/**
 * Given a PPM image and a string message, encodes the message in the image.
 * The function will exit with an error message if the given string is too long for the image IE more characters than pixels.
 */
PPM encodePPM(char* message, PPM image){
	int pixelcount = image.height*image.width, pxlindex = 0, msgindex = 0, msglen = strlen(message);
	char temp;

	srand(pixelcount);

	if (msglen > pixelcount){
		printf("Message too large for selected image");
		exit(0);
	}

	for(;msgindex<msglen;msgindex++){ //Iterates over each character in the message
			do{
				pxlindex = rand()%pixelcount;
			} while (image.pixellist[pxlindex].dirty);	//Continually tries to find a 'clean' pixel

			image.pixellist[pxlindex] = encodechar(image.pixellist[pxlindex], message[msgindex], image.maxval); //Updates the selected pixel
	}

	return image;

}

/**
 * Note that output from encodePPM should not be directly passed to this function, as this function assumes the dirty bit for each pixel is 0 as provided by parsefile.
 */
char * decodePPM(PPM original, PPM encoded){
	int pixelcount, pxlindex = 0, msgindex =0;

	if (original.height != encoded.height || original.width != encoded.width || original.maxval != encoded.maxval){
		printf("Fatal file mismatch");
		exit(0);
	}

	pixelcount = encoded.height*encoded.width;

	srand(pixelcount);

	char * message = (char *) malloc(pixelcount*(sizeof(char)));

	for(; msgindex<pixelcount;msgindex++){
		do{
			pxlindex = rand()%pixelcount;
		} while (encoded.pixellist[pxlindex].dirty); //Constantly searches for a pixel which hasn't been read from yet.

		if(original.pixellist[pxlindex].blue == encoded.pixellist[pxlindex].blue){
			break; //If the two pixels are the same then the message end must have been reached.
		}

		encoded.pixellist[pxlindex].dirty = 1;
		message[msgindex] = decodepixel(original.pixellist[pxlindex], encoded.pixellist[pxlindex], encoded.maxval);
	}

	return message;

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

//showPPM(encodePPM("Hp", parsefile(argv[1])));

	printf("%s", decodePPM(parsefile(argv[1]), parsefile(argv[2])));

}
