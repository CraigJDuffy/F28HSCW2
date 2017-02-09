/*
 * parser.c
 *
 *  Created on: 28 Jan 2017
 *      Author: Tommy
 */

#include <stdio.h>
#include <stdlib.h> //For exit and malloc.
#include <string.h> //For strlen and strcpy
#include <unistd.h> //For getopt
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

int verbose = 0;

/**
 * A preliminary function which given a filepath will parse and validate a PPM file.
 */
PPM parsefile(FILE * fin) {
	PPM ppmfile;
	Node * listindex;
	char linein[1000];
	regex_t regex;
	int int1, int2, int3, err, pixelcount;

	if (verbose) printf("Parsing file\n");

	fgets(linein, 999, fin);
	if(verbose) printf("%s", linein);

	if (applyregex("P3", linein)) { //Checks first line of file is "P3"
	/*throw new*/formatexception();
	}

	ppmfile.commentlist = (Node *) malloc(sizeof(Node));
	listindex = ppmfile.commentlist; //Both point to the empty list node

	if (fgets(linein, 999, fin) == NULL) { //If EOF is hit (or an error)
			formatexception();
		}
	
	if(verbose) printf("%s", linein);

	while (applyregex("#.*", linein) == 0) {	//Read lines until the first non comment line is reached

		listindex->comment = (char *) malloc(strlen(linein));//Space must be allocated for the next line to work, otherwise malloc memory gets corrupted.
																//Probably from trying to copy the string into unallocated space
		strcpy(listindex->comment,linein);	//Copies the string (char *) linein into the comment string(char *).Straight up comment=linein doesn't work, as the address of linein doesn't ever change but the referenced string does.
											//In other words all copies of comment will point to linein, whose value changes, such that all comments become the last thing stored in linein.
		listindex->next = (Node *) malloc(sizeof(Node));
		listindex = listindex->next;
		listindex->next = NULL; //There is always an empty Node in the list, even if there are no comment lines.
		if (fgets(linein, 999, fin) == NULL) { 	//If EOF is hit (or an error)
			formatexception();
		}
		if(verbose) printf("%s", linein);
		
	} 

	if (sscanf(linein, "%d %d", &ppmfile.width, &ppmfile.height) != 2) { //The Comment regex loop reads the first line after the comments into linein, hence scan on string.
		formatexception(); //They are the right way round; width then height.
	}

	if (ppmfile.width <= 0 || ppmfile.height <= 0) {
		formatexception();
	}

	ppmfile.pixellist = (Pixel *)malloc((ppmfile.width*ppmfile.height)*sizeof(Pixel));

	fgets(linein, 999, fin);
	if(verbose) printf("%s", linein);
	sscanf(linein, "%d", &ppmfile.maxval); //reads in max value. Should discard other characters on the same line
	if (ppmfile.maxval <= 0) {
		formatexception();
	}

	err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Read in the first three RGB values; 1 pixel.
	pixelcount = 0;
	while (err == 3) { //While 3 values have successfully been read.
		if (int1 > ppmfile.maxval || int2 > ppmfile.maxval || int3 > ppmfile.maxval || int1 < 0 || int2 < 0 || int3 < 0 || pixelcount >= (ppmfile.width * ppmfile.height)) { //Check for validity
			formatexception();
		}
		Pixel temp = {int1, int2, int3, 0};
		ppmfile.pixellist[pixelcount] =temp;
		if(verbose) printf("%d: \t%d %d %d\n",pixelcount, ppmfile.pixellist[pixelcount].red ,ppmfile.pixellist[pixelcount].green, ppmfile.pixellist[pixelcount].blue);
		pixelcount++;
		err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Try to read next three values

	}

	if (err == EOF) {
		if(verbose){
			printf("End of File\n\n");
			printf("Press enter to continue\n\n");
			getchar();
		}
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
	
	if(verbose){
		printf("\nDisplaying PPM\n\n");
		printf("Press enter to continue\n\n");
		getchar();
	}

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
	int pixelcount = image.height*image.width, pxlindex = 0, msgindex = 0, msglen;
	char temp;

	if(verbose) printf("Encoding message \"%s\"\n", message);

	msglen = strlen(message);
	srand(pixelcount);

	if (msglen > pixelcount){
		printf("Message too large for selected image");
		exit(0);
	}

	for(;msgindex<msglen;msgindex++){ //Iterates over each character in the message
			do{
				pxlindex = rand()%pixelcount;
			} while (image.pixellist[pxlindex].dirty);	//Continually tries to find a 'clean' pixel

			if(verbose) printf("Encoding pixel %d\n", pxlindex);
			image.pixellist[pxlindex] = encodechar(image.pixellist[pxlindex], message[msgindex], image.maxval); //Updates the selected pixel
	}

	return image;

}

/**
 * A function which given two PPM 'objects' will decode the message. Behaviour on identical images is as yet undefined
 * Note that output from encodePPM should not be directly passed to this function, as both rely on the pixel dirty bit for different purposes.
 */
char * decodePPM(PPM original, PPM encoded){
	int pixelcount, pxlindex = 0, msgindex =0;

	if(verbose) printf("Decoding image\n");

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
			if(verbose) printf("End of message at pixel %d\n", pxlindex);
			break; //If the two pixels are the same then the message end must have been reached.
		}

		encoded.pixellist[pxlindex].dirty = 1; //Has been read from
		if(verbose) printf("Decoding pixel %d\n", pxlindex);
		message[msgindex] = decodepixel(original.pixellist[pxlindex], encoded.pixellist[pxlindex], encoded.maxval);
		message[msgindex+1] = '\0';
	}

	if(verbose) printf("Message decoded\n\n");

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
 * A somewhat superfluous function for opening a file for reading, with built in error checking and 'handling'
 */
FILE * openf(char * filepath){
	FILE * f;

	if(verbose) printf("Opening file \"%s\"\n", filepath);

	f = fopen(filepath, "r");

	if (f==NULL){
		printf("Unable to open file: %s\n", filepath);
		exit(0);
	}

	return f;
}

/**
 * Function reports a file format exception before exiting.
 * Another superfluous function which can be accused of pointlessness, other than cutting down of repetition of the two lines in parsefile.
 */
formatexception() {
	printf("\nFile format exception\n");
	exit(0);
}


main(int argc, char ** argv) {
int opt, enc =0, dec =0, mes =0, check =0;
/*
char * encodedfile, message;
This doesn't work, the compiler thinks message is an integer. However the following does work. Why?
*/
char * encodedfile;
char * message;

opterr=0;

while((opt=getopt(argc, argv, "hced:vm:")) != -1){
	switch (opt){
	case 'e':
		enc=1;
		break;
	case 'd':
		dec=1;
		encodedfile = optarg;
		break;
	case 'm':
		mes = 1;
		message = optarg;
		break;
	case 'v':
		verbose=1;
		break;
	case 'c':
		check=1;
		verbose=1;
		break;
	case 'h':
		printf("Usage:\nsteg [-m <message>] [-v] <-e | -d <encoded file> | -c> <base PPM file>\n-e = encode\n-d \e[4mencoded file\e[0m = decode the specified file\n-m \e[4mmessage\e[0m = specify message\n-v = verbose\n-c = check file\n");
		exit(0);
	case '?':
		switch (optopt){
		case 'd':
			printf("Missing option-argument:\n-d \e[4mencodedfile\e[0m = decode the specified file\n");
			break;
		case 'm':
			printf("Missing option-argument:\n-m \e[4mmessage\e[0m = specify message\n");
			break;
		default:
			printf("Unidentified option: %c\n", optopt);
			break;
		}
		exit(0);
	}
}

if (optind==argc){
	printf("Missing argument for base PPM file\n");
	exit(0);
}

if (enc + dec + check > 1){
	printf("Options -e, -c, and -d are mutually exclusive.\n");
	exit(0);
}else if ((enc+dec+check)==0){
	printf("Must specify mode of operation:\nsteg [-m <message>] [-v] <-e | -d <encoded file> | -c> <base PPM file>\n-e = encode\n-d \e[4mencoded file\e[0m = decode the specified file\n-m \e[4mmessage\e[0m = specify message\n-v = verbose\n-c = check file\n");
	exit(0);
} else if (enc){
	if (mes){
		showPPM(encodePPM(message, parsefile(openf(argv[optind]))));
	}
	else {
		//Get message, show it
	}
} else if (dec){
	printf("Message reads:\n%s\n", decodePPM(parsefile(openf(argv[optind])), parsefile(openf(encodedfile))));
} else if (check) {
	parsefile(openf(argv[optind]));
	printf("File is valid\n\n");
}
//showPPM(encodePPM("Hp", parsefile(openf(argv[1]))));

	//printf("%s", decodePPM(parsefile(openf(argv[1])), parsefile(openf(argv[2]))));

}
