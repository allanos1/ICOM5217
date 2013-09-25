
			.global test
test:		
;---------------------------------------------------------------------
; Initialization Code
;---------------------------------------------------------------------	
			MOV 	R1, #0x5400		; R1 Contains GPIOPFDIR
			MOVT	R1, #0x4002		
			MOV		R2, #0x551C 	; R2 Contains GPIOPFDEN
			MOVT 	R2, #0x4002
			MOV		R3, #0x53FC		; R3 Contains GPIOPFDATA
			MOVT	R3, #0x4002	
			MOV 	R4, #0x0E		; Value for both DIR and DEN
			MOV		R5, #0x0A		;
			MOV		R6, #0x01		;
			STR		R5, [R3]
			STR 	R4, [R1]		; Store the value 0x08 in DIR: enable output.
			STR 	R4, [R2]		; Store the value 0x08 in DEN: enable digital function.
			
main_loop: 	MOV 	R8, #0xFFFF		;
			MOVT	R8, #0x0000		;
			LDR		R7, [R3]
			
;---------------------------------------------------------------------
; Begin if/else cases.
;---------------------------------------------------------------------
			CMP		R9, #0x00
			BEQ		turn_off
			CMP		R6, #0x01
			BEQ		turn_red
			CMP		R6, #0x02
			BEQ		turn_blue
			CMP		R6, #0x03
			BEQ		turn_green
turn_red:	EOR 	R7, #0x08		;
			MOV		R6, #0x02
			MOV		R9, #0x00
			B		end_turn
turn_blue:  EOR		R7, #0x04		;
			MOV		R6, #0x03
			MOV		R9, #0x00
			B		end_turn
turn_green: EOR		R7, #0x02		;
			MOV		R6, #0x01
			MOV		R9, #0x00
			B		end_turn
turn_off:	MOV		R7, #0x00		;
			MOV		R9, #0x01
			B		end_turn

end_turn:	STR		R7, [R3]		; Update the GPIODATA

sub_loop:	SUB 	R8, #0x0001	
			CMP		R8, #0
			BNE 	sub_loop
			B 		main_loop
			NOP