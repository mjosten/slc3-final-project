; Michael Josten
; Final Project test code TCSS 372 
; This test code will test out the stack instructions
; start result: R2 = 5, R3 = 7, R4 = 9
; End result: R2 = 9, R3 = 5, R4 = 7

		.ORIG x3000
		ADD R2, R2, #5
		ADD R3, R3, #7
		ADD R4, R4, #9
		LEA R6, STACKPOINTER
		.FILL x0	;push R2
		.FILL x0	;push R3
		.FILL x0	;push R4
		.FILL x0	;pop R2
		.FILL x0	;pop R4
		.FILL x0	;pop R3


		HALT
		.BLKW 20
STACKPOINTER	.FILL #0
		.END