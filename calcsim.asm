; Michael Josten
; Final Project TCSS 372
; Assembly program for the final project which will a as a very...very simple calculator. Only addition will be implemented
; Program will prompt user for two numbers and those numbers will be added 
; becuase of the limitation, can only output a char from 0-9
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
		LD R1, NEGZERO		; set R1 to negative ascii zero

	; prompt and push first result
		LEA R0, PROMPT1		; print prompt 1
		PUTS			
		GETC			; get user input
		OUT
		ADD R0, R1, R0		; subtract x30 from R0
		JSR PUSH

	; prompt and push second result
		LEA R0, PROMPT2		; Print Prompt 2
		PUTS
		GETC
		OUT
		ADD R0, R1, R0		; subtract x30 from R0
		JSR PUSH

	; display sum of first and second result
		LEA R0, PROMPT3		; Print Prompt 3
		PUTS
		JSR SUM 		; calls sum routine which calls TOASCII routine which will
					; make the result an ascii character
		
		OUT
		HALT

	; SUM Routine pops 2 from the stack and adds them together and 
	; returns the sum to R0
SUM		AND R4, R4, #0
		ADD R4, R7, #0	; temp storage for return address becuase need to pop first
		JSR POP		; store first pop in R1
		AND R1, R1, #0 	; set R1 to zero
		ADD R1, R0, R1	; set R1 to R0 (first pop)

	; Pop second result
		JSR POP		

	; sum result and store in R0, return register
		ADD R2, R1, R0	; Sum R1 and R0
		; put R7 into R0 then push it then push the R2 onto stack
		; push result and R7 onto stack then call TOASCII routine
		AND R0, R0, #0
		ADD R0, R4, #0	; put R4(temp return address) into R0 then push
		JSR PUSH
		AND R0, R0, #0
		ADD R0, R2, #0	; put result into R0 then push
		JSR PUSH
		JSR TOASCII
		AND R2, R2, #0	
		ADD R2, R0, #0	; put result from TOASCII in R2
		JSR POP		; POP return address
		AND R7, R7, #0
		ADD R7, R0, #0	; save return address in R7 from R0
		AND R0, R0, #0	
		ADD R0, R2, #0	; put the result into R0
		RET

	; Routine that will add x30 to 1st argument which will be obtained by poping the stack
	; return value is R0
TOASCII		AND R4, R4, #0
		ADD R4, R7, #0 	; temp storage for R7 becuase call pop
		JSR POP
		LD R1, ZERO
		ADD R0, R1, R0
		AND R7, R7, #0
		ADD R7, R4, #0	; reinstate R7
		RET



	; POP routine with underflow check
POP		ST R2, SAVER2	
		LEA R2, STACKBASE	; load R2 with stackbase address to compare to R6
		NOT R2, R2		; 2's comp R2
		ADD R2, R2, #1
		ADD R2, R2, R6		; compare R6(stackpointer) to stackbase
	; if result is zero, stack is empty and result is a zero where R5 needs to be set to 1
		BRz STACKFAIL
	; if result is not zero, initiate pop.
		LDR R0, R6, #0		; actual pop to be replaced by instruction
		ADD R6, R6, #1
		AND R5, R5, #0		; set R5 to zero to signify success
		LD R2, SAVER2
		RET
		

	; PUSH routine, R0 onto stack and will check for overflow
PUSH		ST R2, SAVER2
		LD R2, STACKMAX		; load R2 with last address location of stack
		NOT R2, R2		; compare stackmax with R6 to determine if overflow
		ADD R2, R2, #1		; 2's comp Stackmax
		ADD R2, R6, R2		; compare stackmax with R6
		BRz STACKFAIL		; if result is zero then overflow
	; success on push, set R5 to zero
		ADD R6, R6, #-1		; actual push that needs to be replaced by instruction
		STR R0, R6, #0		
		AND R5, R5, #0		; set R5 to zero
		LD R2, SAVER2
		RET
		
STACKFAIL	AND R5, R5, #0		; set R5 to one
		ADD R5, R5, #1
		LD R2, SAVER2
		RET

; Variables and stack for program
STACKMAX	.BLKW 9
STACKBASE	.FILL x0000		; stack will be 10 spaces large
SAVER2		.FILL x0000		; space to save R2 register
PROMPT1		.STRINGZ "Enter the first number to sum: " ; first prompt
PROMPT2		.FILL x000A		; Second Prompt for user input
		.STRINGZ "Enter the second number to sum: "
PROMPT3		.FILL x000A		; Third prompt 
		.STRINGZ "Sum result: " 
NEGZERO		.FILL x-0030		; negative ascii value of zero
ZERO		.FILL x0030		; ascii value of zero
		.END