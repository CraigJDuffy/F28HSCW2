#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define BlockSize (4*1024)
#define GPIO_Base 0x3f200000

#define LEDSel GPIO_GPFSEL0
#define LEDSet GPIO_GPSET0
#define LEDClr GPIO_GPCLR0
#define LEDSelBit 18
#define LEDSetBit 6

#define GPIO_GPFSEL0 0
#define GPIO_GPSET0 7
#define GPIO_GPCLR0 10

volatile unsigned int * gpio;
volatile unsigned int tim;

digitalWrite(volatile int * gpio, int pin, int state){
		asm(
		"	MOV R0, %[GPIO]\n"
		"	ADD R0, #28  @Move gpio base upto GPSET0\n"
		"	MOV R1, %[state]\n"
		"	CMP R1, #1\n"
		"	BEQ set\n"
		"	ADD R0, #12 @move GPIO up to GPCLR0\n"
		"	set: MOV R1, %[pin]\n"
		"	CMP R1, #32\n"
		"	BLT write @Pin number is offset and GPIO is correct register\n"
		"	ADD R0, #4\n @Move to next register"
		"	SUB R1, #32\n @Fix pin number to offset"
		"	write: MOV R2, #1\n"
		"	LDR R3, [R0]\n"
		"	ORR R2, R3, R2, LSL R1\n"
		"	STR R2, [R0]\n"
		:[pin] "+r" (pin)
		:[GPIO] "r" (gpio), [state] "r" (state)
		:"r0","r1", "r2", "r3", "cc"
		);
}


int main (void){
	int fd;
	
	if ( (fd = open("/dev/mem",O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
		printf("cannot open /dev/main\n");
		exit(0);
	}
	
	gpio = (uint32_t *) mmap(0, BlockSize, PROT_READ | PROT_WRITE , MAP_SHARED, fd, GPIO_Base);
	
	if ((int) gpio ==-1 ){
		printf("Can't mmap\n");
		exit(0);
	}
	
	gpio[LEDSel] |= (1<<LEDSelBit);
	
	while(1){
		digitalWrite(gpio, 6, 0);
		usleep(100000);
		digitalWrite(gpio, 6, 1);
	}
	
}
