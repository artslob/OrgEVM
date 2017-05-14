dseg at 0x10
	qwe: ds 1

cseg at 0x00
	jmp start
	
start:
	dec 
	
	mov a, #0
	jz start
	END