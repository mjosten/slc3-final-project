.ORIG x3000
		LEA 	R6,StackBase
		ADD 	R6,R6,#-1
		LEA 	R0,PromptMsg
		PUTS
		GETC
		OUT

Test 		LD 	R1,NegX
		ADD 	R1,R1,R0
		BRz 	Exit

		LD 	R1,NegC
		ADD 	R1,R1,R0
		BRz 	OpClear

		LD 	R1,NegPlus
		ADD 	R1,R1,R0
		BRz 	OpAdd

		LD 	R1,NegMult
		ADD 	R1,R1,R0
		BRz 	OpMult

		LD 	R1,NegMinus
		ADD 	R1,R1,R0
		BRz 	OpNeg

		LD 	R1,NegD 
		ADD 	R1,R1,R0
		BRz 	OpDisplay 

		BRnzp 	PushValue 

NewCommand 	LEA 	R0,PromptMsg
		PUTS
		GETC
		OUT
		BRnzp 	Test
Exit 		HALT

PromptMsg 	.FILL 	xOOOA
		.STRINGZ "Enter a command: "
NegX 		.FILL 	XFFA8
NegC 		.FILL 	xFFBD
NegPlus 	.FILL 	XFFD5
NegMinus 	.FILL 	XFFD3
NegMult 	.FILL 	XFFD6
NegD 		.FILL 	xFFBC

PushValue 	LEA 	R1,ASCIIBUFF 
		LD 	R2,MaxDigits
 
ValueLoop 	ADD 	R3,R0,xFFF6
		BRz 	Goodlnput
		ADD 	R2,R2,#0
		BRz 	TooLargeInput
		ADD 	R2,R2,#-1 
		STR 	R0,R1,#0 
		ADD 	R1,R1,#1
		GETC
		OUT
		BRnzp 	ValueLoop

Goodlnput 	LEA 	R2,ASCIIBUFF
		NOT 	R2,R2
		ADD 	R2,R2,#1
		ADD 	R1,R1,R2 
		JSR 	ASCIItoBinary
		JSR 	PUSH
		BRnzp 	NewCommand

TooLargeInput 	GETC 
		OUT
		ADD 	R3,R0,xFFF6
		BRnp 	TooLargelnput
		LEA 	R0,TooManyDigits
		PUTS
		BRnzp 	NewCommand

TooManyDigits 	.FILL 	xOOOA
		.STRINGZ "Too many digits"
MaxDigits 	.FILL 	x0003

POP 		LEA 	R0,StackBase
		NOT 	R0, R0
		ADD 	R0,R0,#2 
		ADD 	R0,R0,R6 
		BRz 	Underflow
		LDR 	R0,R6,#0 
		ADD 	R6,R6,#1 
		AND 	R5,R5,#0
		RET

Underflow 	ST 	R7,Save
		LEA 	R0,UnderflowMsg
		PUTS
		LD 	R1,Save 
		AND 	R5,R5,#0
		ADD 	R5,R5,#1
		RET

Save 		.FILL 	xOOOO
StackMax 	.BLKW 	9
StackBase 	.FILL 	xOOOO
UnderflowMsg 	.FILL 	X000A
		.STRINGZ "Error: Too Few Values on the Stack."

.END