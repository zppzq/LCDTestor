$include (c8051f340.inc)               ; Include register definition file

MMC_COMMAND SEGMENT CODE
RSEG MMC_COMMAND

PUBLIC _SD_READ_BYTES_ASM
PUBLIC _SD_WRITE_BYTES_ASM

;///////////////////////////////
;/// function SD_READ_BYTES_ASM
;///////////////////////////////

_SD_READ_BYTES_ASM:
; r1,r2,r3 - pchar - pointer to data buffer
; r4,r5 - buffer length
		MOV		DPH,R2
		MOV   	DPL,R1

		MOV		A,R5
		JNZ		BEGIN_READ
		MOV		A,R4
		JZ		FINISH_COPY
		DEC		R4

BEGIN_READ:		
		MOV		SPI0DAT,#0xFF		

COPY_LABEL:
		JNB   	SPIF,$
		CLR		SPIF
		MOV		A,SPI0DAT				; read spi byte
		MOVX	@DPTR,A
		
		
		DJNZ	R5,NO_DEC_HI_COUNTER


DEC_HI_COUNTER:
		
		MOV		A,R4		
		JZ		FINISH_COPY 	
		DEC		R4

NO_DEC_HI_COUNTER:
			
		MOV		SPI0DAT,#0xFF	
		INC		DPL
		MOV		A,DPL
		JNZ		COPY_LABEL
		INC		DPH
		SJMP	COPY_LABEL

FINISH_COPY:

		RET

;///////////////////////////////
;/// END of function SD_READ_BYTES_ASM
;///////////////////////////////

;///////////////////////////////
;/// function SD_WRITE_BYTES_ASM
;///////////////////////////////


_SD_WRITE_BYTES_ASM:
		MOV		DPH,R2
		MOV   	DPL,R1
	
WRITE_COPY_LABEL:		
		MOV		A,R5
		DEC		R5
		JNZ		WRITE_DATA	
		MOV		r5,#0xff
		MOV		A,R4			
		JZ		WRITE_FINISH_COPY 
		DEC		R4

WRITE_DATA:
		MOVX	A,@DPTR
		MOV		SPI0DAT,A		
		JNB   	SPIF,$
		CLR		SPIF

		INC		DPL
		MOV		A,DPL
		JNZ		WRITE_DEC_HI_COUNTER
		INC		DPH

WRITE_DEC_HI_COUNTER:	
		SJMP	WRITE_COPY_LABEL

WRITE_FINISH_COPY:
		RET

;////////////////////////////////
;/// END of function SD_WRITE_BYTES_ASM
;////////////////////////////////




END