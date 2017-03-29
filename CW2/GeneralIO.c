#ifndef GENERALIO_H
#define GENERALIO_H

#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


#define BlockSize (4*1024)
#define GPIO_Base 0x3f200000

#define GPFSEL_OUTPUT 0x01
#define GPFSEL_INPUT 0x00

#define BUTTON 16
#define TIMEOUT 8000


/*
 * Inline Assembly method for setting the function of a BCM pin
 * */
pinMode(volatile int * gpio, int pin, unsigned char function){
	int GPFSEL = (pin/10)*4;
	int bitshift = (pin%10)*3;
	function = function & 0x07;
	asm(
	"	MOV R0, %[GPIO]\n"
	"	MOV R1, %[bitshift]\n"
	"	MOV R2, %[function]\n"
	"	ADD R0, %[GPFSEL]\n"
	"	LDR R3, [R0]\n"
	"	here:MOV R4, #7 @prepare 111\n"
	"	MVN R4, R4, LSL R1 @shift and negate 111\n"
	"	AND R3, R3, R4\n"
	"	ORR R2, R3, R2, LSL R1\n"
	"	STR R2, [R0]\n"
	:[function] "+r" (function)
	:[GPIO] "r" (gpio), [bitshift] "r" (bitshift), [GPFSEL] "r" (GPFSEL)
	:"r0", "r1", "r2", "r3", "r4", "cc"
	);
}


/*
 * Inline assembly function to set a BCM pin high or low
 * */
digitalWrite(volatile int * gpio, int pin, int state){
		asm(
		"	alovelylabel:\n"
		"	MOV R0, %[GPIO]\n"
		"	ADD R0, #28  @Move gpio base upto GPSET0\n"
		"	MOV R1, %[state]\n"
		"	CMP R1, #1\n"
		"	BEQ set\n"
		"	ADD R0, #12 @move GPIO up to GPCLR0\n"
		"	set: MOV R1, %[pin]\n"
		"	CMP R1, #32\n"
		"	BLT write @Pin number is offset and GPIO is correct register\n"
		"	ADD R0, #4 @Move to next register\n"
		"	SUB R1, #32 @Fix pin number to offset\n"
		"	write: MOV R2, #1\n"
		"	LSL R2, R1\n"
		"	STR R2, [R0]\n"
		:[pin] "+r" (pin)
		:[GPIO] "r" (gpio), [state] "r" (state)
		:"r0","r1", "r2", "r3", "cc"
		);
}


/*Inline Assembly function readPin*/
int readPin (int pin) {
    int offset = ((pin / 32) + 13) * 4;
    int pinSet = pin % 32;
    int r;
    asm(
        "\tLDR R0, %[gpi]\n"
        "\tMOV R1, R0\n"
        "\tADD R1, %[offset]\n"
        "\tLDR R1, [R1]\n"
        "\tMOV R2, #1\n"
        "\tLSL R2, %[pinShift]\n"
        "\tAND %[r], R2, R1\n"
        : [r] "=r" (r)
        : [gpi] "m" (gpio),
          [offset] "r" (offset),
          [pinShift] "r" (pinSet)
        : "r0", "r1", "r2", "cc", "memory"
    );

    if (r != 0)
      return 1;
    return 0;
}


/*
 * Convenience function handling the memory mapping of GPIO
 * */
volatile int * getGPIO(){
	volatile int * gpio;
	int fd;


	if ( (fd = open("/dev/mem",O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
		printf("cannot open /dev/main\n");
		exit(0);
	}

	gpio = mmap(0, BlockSize, PROT_READ | PROT_WRITE , MAP_SHARED, fd, GPIO_Base);

	if ((int) gpio ==-1 ){

		exit(0);
	}

	return gpio;
}

#endif
