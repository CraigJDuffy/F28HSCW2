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

#define HELPSTRING "steg <-e | -c | -d <encoded file>> <base PPM file> [-m <message>] [-o <output file>] [-v]\n-d \e[4mencoded file\e[0m = decode the specified file\n-m \e[4mmessage\e[0m \t= specify message\n-o \e[4moutput file\e[0m \t= specify output file\n-e \t\t= encode\n-v \t\t= verbose\n-c \t\t= check file\n"

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
int redirect =0;
FILE * streamout;

/**
 * A function to parse and load for use a simple PPM plain text file.
 * Memory allocated in this function can be freed using the freePPM function.
 */
PPM parsefile(FILE * fin) {
	PPM ppmfile;
	Node * listindex;
	char linein[1000];
	regex_t regex;
	int int1, int2, int3, err, pixelcount;

	if (verbose) fprintf(streamout, "Parsing file\n");

	fgets(linein, 999, fin);
	if(verbose) fprintf(streamout, "%s", linein);

	if (applyregex("^P3\r*$", linein)) { //Checks first line of file is "P3" and only P3. "\r*" required to handle windows/unix differences
	/*throw new*/formatexception();
	}

	ppmfile.commentlist = (Node *) malloc(sizeof(Node));
	listindex = ppmfile.commentlist; //Both point to the empty list node

	if (fgets(linein, 999, fin) == NULL) { //If EOF is hit (or an error)
			formatexception();
		}
	
	if(verbose) fprintf(streamout, "%s", linein);

	while (applyregex("^#.*$", linein) == 0) {	//Read lines until the first non comment line is reached. Note "\r*" not required as "." matches "\r"

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
		if(verbose) fprintf(streamout, "%s", linein);
		
	} 

	if (sscanf(linein, "%d %d", &ppmfile.width, &ppmfile.height) != 2) { //The Comment regex loop reads the first line after the comments into linein, hence scan on string.
		formatexception(); //They are the right way round; width then height.
	}

	if (ppmfile.width <= 0 || ppmfile.height <= 0) {
		formatexception();
	}

	ppmfile.pixellist = (Pixel *)malloc((ppmfile.width*ppmfile.height)*sizeof(Pixel));

	fgets(linein, 999, fin);
	if(verbose) fprintf(streamout, "%s", linein);
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
		if(verbose) fprintf(streamout, "%d: \t%d %d %d\n",pixelcount, ppmfile.pixellist[pixelcount].red ,ppmfile.pixellist[pixelcount].green, ppmfile.pixellist[pixelcount].blue);
		pixelcount++;
		err = fscanf(fin, "%d %d %d", &int1, &int2, &int3); //Try to read next three values

	}

	if (err == EOF) {
		if(verbose){
			fprintf(streamout, "End of File\n\n");
			if(!redirect) printf("Press enter to continue\n\n");
			if(!redirect) getchar(); //Only pause for input when outputting to stdout.
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
 * Given a PPM image the function consumes it and prints the corresponding file format to the file stream specified in global variable streamout.
 * The function defaults to standard out, though the user may change this with command line option invocation.
 *
 */
showPPM(PPM image){
	Node * listindex;
	int pixelcount=0;
	
	if(verbose){
		fprintf(streamout, "\nDisplaying PPM\n\n");
		if(!redirect)printf("Press enter to continue\n\n");
		if(!redirect)getchar(); //Only pause for input when outputting to stdout.
	}

	fprintf(streamout, "P3\n");
	
	listindex=image.commentlist;
	while(listindex->next != NULL){
		fprintf(streamout, "%s", listindex->comment);
		listindex = listindex->next;
	}
	
	fprintf(streamout, "%d %d\n", image.width, image.height);
	fprintf(streamout, "%d\n", image.maxval);
	
	for(pixelcount=0;pixelcount<image.width*image.height;pixelcount++)
		fprintf(streamout, "%d %d %d\n", image.pixellist[pixelcount].red, image.pixellist[pixelcount].green, image.pixellist[pixelcount].blue);

	freePPM(image); //This handles freeing memory when encoding.
}



/**
 * Encodes a given integer into a Pixel format.
 * The integer is encoded as an absolute value for the red value, and as an offset for the green value.
 * The encoding provides for (maxval+1)*(maxval) possible values.
 * The function does not check the cleanliness of the given pixel.
 */
Pixel encodechar(Pixel original, unsigned int character, int maxval) {
	int rcode, gdiff;
	unsigned int c = character;

	rcode = c % (maxval + 1);
	gdiff = c / (maxval + 1); //Truncated towards 0. If maxval is negative the effect is ceiling, not floor.

	if ((gdiff) > maxval) {
		fprintf(streamout, "Unable to encode character %c with max colour value of %d\n", character, maxval);
		if(redirect) printf("Unable to encode character %c with max colour value of %d\n", character, maxval);
		exit(0);
	}

	Pixel encoded = {rcode, ((original.green + gdiff) % (maxval + 1)), (original.blue +1) % (maxval+1), 1}; //1 is added to the blue field so that dirty pixels can always be identified when compared with the original

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

	if(verbose) fprintf(streamout, "Encoding message \"%s\"\n", message);

	msglen = strlen(message);
	srand(pixelcount);

	if (msglen > pixelcount){
		fprintf(streamout, "Message too large for selected image\n");
		if(redirect) printf("Message too large for selected image\n");
		exit(0);
	}

	for(;msgindex<msglen;msgindex++){ //Iterates over each character in the message
			do{
				pxlindex = rand()%pixelcount;
			} while (image.pixellist[pxlindex].dirty);	//Continually tries to find a 'clean' pixel

			if(verbose) fprintf(streamout, "Encoding pixel %d\n", pxlindex);
			image.pixellist[pxlindex] = encodechar(image.pixellist[pxlindex], message[msgindex], image.maxval); //Updates the selected pixel
	}

	return image;

}

/**
 * A function which given two PPM 'objects' will consume them and decode the message. Behaviour on identical images is as yet undefined
 * Note that output from encodePPM should not be directly passed to this function, as both rely on the pixel dirty bit for different purposes.
 */
char * decodePPM(PPM original, PPM encoded){
	int pixelcount, pxlindex = 0, msgindex =0;

	if(verbose) fprintf(streamout, "Decoding image\n");

	if (original.height != encoded.height || original.width != encoded.width || original.maxval != encoded.maxval){
		fprintf(streamout, "Fatal file mismatch\n");
		if(redirect) printf("Fatal file mismatch\n");
		exit(0);
	}

	pixelcount = encoded.height*encoded.width;

	srand(pixelcount);

	char * message = (char *) malloc(pixelcount*(sizeof(char))); //The absolute maximum size possible for a message
	//Memory freed in calling if statement in main method.

	for(; msgindex<pixelcount;msgindex++){
		do{
			pxlindex = rand()%pixelcount;
		} while (encoded.pixellist[pxlindex].dirty); //Constantly searches for a pixel which hasn't been read from yet.

		if(original.pixellist[pxlindex].blue == encoded.pixellist[pxlindex].blue){
			if(verbose) fprintf(streamout, "End of message at pixel %d\n", pxlindex);
			break; //If the two pixels are the same then the message end must have been reached.
		}

		encoded.pixellist[pxlindex].dirty = 1; //Has been read from
		if(verbose) fprintf(streamout, "Decoding pixel %d\n", pxlindex);
		message[msgindex] = decodepixel(original.pixellist[pxlindex], encoded.pixellist[pxlindex], encoded.maxval);
		message[msgindex+1] = '\0';
	}

	if(verbose) fprintf(streamout, "Message decoded\n\n");

	freePPM(original);
	freePPM(encoded);//Free the memory of the PPM images, now that they're no longer needed.

	return message;

}

/**
 * A method for handling memory deallocation for the internals of a PPM image.
 * Note that this function does not free the memory directly represented by image, just the memory to which the image attributes point.
 */
freePPM(PPM image){
	Node * listindex;
	Node * delindex;

	listindex=image.commentlist;
	delindex=image.commentlist;
	while(listindex->next != NULL){
		free(listindex->comment);
		listindex = listindex->next;
		free(delindex);
		delindex=listindex;
	}

	free(delindex); //Frees the last, empty node in the list. Such a node is not deleted in the loop due to the next field being null and the loop terminating.

	free(image.pixellist);
}

/**
 * Function applies a given regex to a string, returning 0 on match or 1 on fail.
 * Function may return other values in the event of internal error. These can be identified using the
 * regerror function in the regex library.
 */
int applyregex(char* regex, char* string) {
	int result;
	regex_t expression;

	if (regcomp(&expression, regex, REG_NEWLINE)) { //compile the regex string into address expression. Return 0 on success.
		fprintf(streamout, "Error compiling regex.\n");
		if(redirect) printf("\nError compiling regex\n"); //Print to stdout in addition to the file
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
FILE * openf(char * filepath, char * mode){
	FILE * f;

	if(verbose) fprintf(streamout, "Opening file \"%s\"\n", filepath);

	f = fopen(filepath, mode);

	if (f==NULL){
		fprintf(streamout, "Unable to open file: %s\n", filepath);
		if(redirect) printf("\nUnable to open file: %s\n", filepath); //Print to stdout in addition to the file
		exit(0);
	}

	return f;
}

/**
 * Function reports a file format exception before exiting.
 * Another superfluous function which can be accused of pointlessness, other than cutting down of repetition of the two lines in parsefile.
 */
formatexception() {
	fprintf(streamout, "\nFile format exception\n");
	if(redirect) printf("\nFile format exception\n"); //Print to stdout in addition to the file
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
char input[5001];

opterr=0;
streamout = stdout;

while((opt=getopt(argc, argv, "hced:vm:o:")) != -1){
	switch (opt){
	case 'h':
		printf("Usage:\n%s", HELPSTRING);
		exit(0);
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
		break;
	case 'o':
		streamout = openf(optarg, "w");
		redirect=1;
		break;
	case '?':
		switch (optopt){
		case 'd':
			printf("Missing option-argument:\n-d \e[4mencodedfile\e[0m = decode the specified file\n");
			break;
		case 'm':
			printf("Missing option-argument:\n-m \e[4mmessage\e[0m = specify message\n");
			break;
		case 'o':
			printf("Missing option-argument:\n-o \e[4moutput file\e[0m = specify output file\n");
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
	printf("Must specify mode of operation:Encode, Decode, or File Check\n%s", HELPSTRING);
	exit(0);
} else if (enc){
	if (mes){
		showPPM(encodePPM(message, parsefile(openf(argv[optind], "r")))); //PPM consumed by showPPM function. No memory freeing required here.
		if(redirect) printf("Message encoded\n");
	}
	else {
		printf("\nMessages longer than 5000 Bytes will be truncated without warning.\n");
		printf("Please enter message to encode >>");
		fgets(input, 5000, stdin); //Note the array is defined as 5001, accommodating the terminating character
		printf("\n");
		showPPM(encodePPM(input, parsefile(openf(argv[optind], "r")))); //PPM consumed by showPPM function. No memory freeing required here.
		if(redirect) printf("Message encoded\n");
	}
} else if (dec){
	message = decodePPM(parsefile(openf(argv[optind], "r")), parsefile(openf(encodedfile, "r"))); //PPM images consumed by decodePPM, no memory freeing required here.
	fprintf(streamout, "\nMessage reads:\n%s\n", message);
	if(redirect) printf("Message decoded\n");
	free(message); //Frees memory from decodePPM
} else if (check) {
	freePPM(parsefile(openf(argv[optind], "r")));
	fprintf(streamout, "\nFile is valid\n");
}


}
