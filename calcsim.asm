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
		LEA R0, PROMPT1		; print prompt 1
		PUTS			
		GETC			; get user input
		JSR PUSH
		LEA R0, PROMPT2		; Print Prompt 2
		PUTS
		GETC
		JSR PUSH
		LEA R0, PROMPT3		; Print Prompt 3
		PUTS
		JSR SUM
		OUT
		HALT

	; SUM Routine pops 2 from the stack and adds them together and 
	; returns the sum to R0
SUM		AND R5, R5, #0
		ADD R5, R7, #0	; Store R7
		;pop twice and add to registers


	; PUSH routine, R0 onto stack and will check for overflow
PUSH		ADD R6, R6, #-1		; Decrement stack pointer
		STR R0, R6, #0		; store in the stack
		;check overflow, compare stackbase address with current address
		RET
		

	; POP routine, load R0 with next number from stack, check for underflow
POP		LDR R0, R6, #0		; load from stack
		ADD R6, R6, #1		; increment stack pointer
		; check underflow
		RET

	

		.BLKW 9
STACKBASE	.FILL x0000		; stack will be 10 spaces large

PROMPT1		.STRINGZ "Enter the first number to sum: " ; first prompt
PROMPT2		.FILL x000A		; Second Prompt for user input
		.STRINGZ "Enter the second number to sum: "
PROMPT3		.FILL x000A		; Third prompt 
		.STRINGZ "Sum result" 
NEGZERO		.FILL x-0030		; negative ascii value of zero
		.END

