.ORIG x3000
		LEA 	R6,StackBase
		ADD 	R6,R6,#-1
		LEA 	R0,PromptMsg
		PUTS
		GETC
		OUT

Test 		LD 	R1,NegX
		ADD 	R1,R1,R0
		BRz 	ExitP

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
ExitP 		HALT

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

RangeCheck 	LD 	R5,Neg999
		ADD 	R4,R0,R5
		BRp 	BadRange
		LD 	R5,Pos999
		ADD 	R4,R0,R5
		BRn 	BadRange
		AND 	R5,R5,#0 
		RET

BadRange 	ST 	R7,Save
		LEA 	R0,RangeErrorMsg
		TRAP 	x22
		LD 	R7,Save
		AND 	R5,R5,#0 
		ADD 	R5,R5,#1
		RET

Neg999 		.FILL 	#-999
Pos999 		.FILL 	#999
RangeErrorMsg 	.FILL xOOOA
		.STRINGZ "Error: Number is out of range."

OpAdd 		JSR	POP
		ADD	R5,R5,#0
		BRp	Exit
		ADD	R1,R0,#0
		JSR	POP
		ADD	R5,R5,#0
		BRp	Restore1
		ADD	R0,R0,R1
		JSR	RangeCheck
		BRp	Restore2
		JSR	PUSH
		RET

Restore2 	ADD	R6,R6,#-1
Restorel 	ADD	R6,R6,#-1
Exit 		RET

OpMult 		AND 	R3,R3,#0
		JSR 	POP
		ADD 	R5,R5,#0
		BRp 	Exit
		ADD 	R1,R0,#0
		JSR 	POP
		ADD 	R5,R5,#0 
		BRp 	Restore1
		ADD 	R2,R0,#0
		BRzp 	PosMultiplier
		ADD 	R3,R3,#1 
		NOT 	R2,R2
		ADD 	R2,R2,#1

PosMultiplier 	AND 	R0,R0,#0
		ADD 	R2,R2,#0
		BRz 	PushMult

MultLoop 	ADD 	R0,R0,R1
		ADD 	R2,R2,#-1
		BRp 	MultLoop
		JSR 	RangeCheck
		ADD 	R5,R5,#0
		BRp 	Restore2
		ADD 	R3,R3,#0	
		BRz 	PushMult
		NOT 	R0,R0 
		ADD 	R0,R0,#1

PushMult 	JSR 	PUSH
		RET

OpNeg 		JSR 	POP
		ADD 	R5,R5,#0
		BRp 	Exit
		NOT 	R0, R0
		ADD 	R0,R0, #1
		JSR 	PUSH

ASCIItoBinary 	AND	R0,R0,#0
		ADD	R1,R1,#0
		BRz	DoneAtoB
		LD	R3,NegASCIIOffset
		LEA	R2,ASCIIBUFF
		ADD	R2,R2,R1
		ADD	R2,R2,#-1
		LDR	R4,R2,#0
		ADD	R4,R4,R3
		ADD	R0,R0,R4
		ADD	R1,R1,#-1
		BRz	DoneAtoB
		ADD	R2,R2,#-1
		LDR	R4,R2,#0
		ADD	R4,R4,R3
		LEA	R5,LookUp10
		ADD	R5,R5,R4
		LDR	R4,R5,#0
		ADD	R0,R0,R4
		ADD	R1,R1,#-1
		BRz	DoneAtoB
		ADD	R2,R2,#-1
		LDR	R4,R2,#0
		ADD	R4,R4,R3
		LEA	R5,LookUp100
		ADD	R5,R5,R4
		LDR	R4,R5,#0
		ADD	R0,R0,R4

DoneAtoB 	RET

NegASCIIOffset 	.FILL 	xFFDO
ASCIIBUFF 	.BLKW 	4
LookUp1O 	.FILL 	#0
		.FILL 	#10
		.FILL 	#20
		.FILL 	#30
		.FILL 	#40
		.FILL 	#50
		.FILL 	#60
		.FILL 	#70
		.FILL	#80
		.FILL 	#90

LookUp1OO 	.FILL 	#0
		.FILL 	#100
		.FILL 	#200
		.FILL 	#300
		.FILL 	#400
		.FILL 	#500
		.FILL 	#600
		.FILL 	#700
		.FILL 	#800
		.FILL 	#900

BinarytoASCII 	LEA 	R1,ASCIIBUFF
		ADD 	R0,R0,#0
		BRn 	NegSign 
		LD 	R2,ASCIIplus
		STR 	R2,R1,#0
		BRnzp 	Begin1OO

NegSign 	LD 	R2,ASCIIminus
		STR 	R2,R1,#0
		NOT 	R0,R0
		ADD 	R0,R0,#1 

BeginlOO 	LD 	R2,ASCIIoffset 
		LD 	R3,Neg1OO 

Loop1OO 	ADD 	R0,R0,R3
		BRn 	End1OO
		ADD 	R2,R2,#1
		BRnzp 	Loop1OO

End1OO 		STR 	R2,R1,#1
		LD 	R3,Pos1OO
		ADD 	R0,R0,R3
		LD 	R2,ASCIIoffset

Begin1O 	LD 	R3,Neg1O 

Loop10 		ADD 	R0,R0,R3
		BRn	End1O
		ADD 	R2,R2,#1
		BRnzp 	Loop10

End10 		STR 	R2,R1,#2
		ADD 	R0,R0,#10

Begin1 		LD 	R2,ASCIIoffset
		ADD 	R2,R2,R0
		STR 	R2,R1,#3
		RET

ASCIIplus 	.FILL 	X002B
ASCIIminus 	.FILL 	X002D
ASCIIoffset 	.FILL 	X0030
Neg1OO 		.FILL 	XFF9C
Pos1OO 		.FILL 	X0064
Neg1O 		.FILL 	XFFF6

.END