; Michael Josten
; Final Project TCSS 372
; Test code for STI
; WIll put 11 (xB) at x3010 using STI instruction


		.ORIG x3000
		AND R4, R4, #0		;set R4 to zero
		ADD R4, R4, #11		;set R4 to 11
		STI R4, TO		; Store 11 at x4000
		HALT
		

TO		.FILL x3010
		
		.END