#ifndef _PiIO_H_
#define _PiIO_H_
//The above is a guard to prevent errors if the header is included more than once.

//Command bit sequences (all flags 0) for Hitachi HD44780U
#define	INST_CLEAR	0x01
#define	INST_HOME	0x02 //NB Home executes Clear
#define	INST_ENTRY	0x04
#define	INST_DISPLAY	0x08
#define	INST_SHIFT	0x10
#define	INST_FUNCSET	0x20
#define	INST_CGADDR	0x40
#define	INST_DDADDR	0x80

//Flag bits for above instructions
#define	ENTRY_SHIFT	0x01
#define	ENTRY_INC	0x02

#define DISPLAY_BLINK 0x01
#define	DISPLAY_CURSOR	0x02
#define	DISPLAY_DISPLAY	0x04

#define SHIFT_RIGHT 0x04
#define SHIFT_DISPLAY 0x08

#define	FUNCSET_FONT	0x04
#define	FUNCSET_ROWS	0x08
#define	FUNCSET_DATALEN	0x10


struct LCDPinSet{
	int strobe, registerSelect;
	int data[4];
} 

struct {
	struct LCDPinSet pins;
	int rows, columns, cursorX, cursorY;
} typedef LCD;

digitalWrite(int gpio, int pin, int state){
	if (state){
		asm(
		"	MOV R0, %[GPIO]\n"
		"	MOV R1, %[pin]\n"
		"	CMP R1, #32\n"
		"	BLT GPSET0\n"
		"	ADD R0, #32\n"
		"	SUB R1, #32\n"
		"	B write\n"
		"	GPSET0: ADD R0, #28\n"
		"	write: MOV R2, #1\n"
		"	LDR R3, [R0]\n"
		"	ORR R2, R3, R2, LSL R1\n"
		"	STR R2, [R0]\n"
		:[pin] "+r" (pin)
		:[GPIO] "r" (gpio)
		:"r0","r1", "r2","cc"
		);
	}
	else {
				asm(
		"	MOV R0, %[GPIO]\n"
		"	MOV R1, %[pin]\n"
		"	CMP R1, #32\n"
		"	BLT GPCLR0\n"
		"	ADD R0, #44\n"
		"	SUB R1, #32\n"
		"	B write\n"
		"	GPCLR0: ADD R0, #40\n"
		"	write: MOV R2, #1\n"
		"	LDR R3, [R0]\n"
		"	ORR R2, R3, R2, LSL R1\n"
		"	STR R2, [R0]\n"
		:[pin] "+r" (pin)
		:[GPIO] "r" (gpio)
		:"r0","r1", "r2","cc"
		);
	}
}

