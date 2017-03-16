


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


