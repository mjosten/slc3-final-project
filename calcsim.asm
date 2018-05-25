; Michael Josten
; Final Project TCSS 372
; Assembly program for the final project which will a as a very...very simple calculator. Only addition will be implemented
; Program will prompt user for two numbers and those numbers will be added 
; together and outputed to user
; Underflow 

; When calling a subroutine, push the arguments onto the stack
; then call JSR
; Then upon return use the value returned by R0

; The called subroutine will pop the arguments off the stack and save them
; in registers then if it calls another subroutine will store R7 onto the stack
; put result in R0 and then pop R7 if it pushed it before
; then return

		.ORIG x3000

	; Main, get input from user, initialize stackbase
		LEA R6, STACKBASE
		LEA R0, PROMPT1	; print prompt 1
		PUTS			
		GETC		; get user input
		JSR PUSH
		HALT


	; PUSH routine, R0 onto stack and will check for overflow
PUSH		RET

	; POP routine, load R0 with next number from stack, check for underflow
POP		RET

	; Convert to Negative routine, will subtract ZERO value from 
CONV2NEG	LD R1, NEGZERO
		; save R7 here
		JSR POP		; R0 will be loaded with user input char
		ADD R0, R0, R1	; subtract R0 with negzero to get int value


		.BLKW 9
STACKBASE	.FILL x0000		; stack will be 10 spaces large
PROMPT1		.STRINGZ "Enter the first number to sum: " ; first prompt
PROMPT2		.FILL x000A		; Second Prompt for user input
		.STRINGZ "Enter the second number to sum: "
NEGZERO		.FILL x-0030		; negative ascii value of zero
		.END

