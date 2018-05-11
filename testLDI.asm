; Michael Josten
; Test LDI code for TCSS 372 Final Project
; Will load 11 using LDI


		.ORIG x3000
		
		LDI R4, TO
		HALT

TO		.FILL x3010
		.BLKW 13
		.FILL xB
		.END