dseg at 0x08
	sec: ds 1
	min: ds 1
		
cseg at 0x00
	jmp start
	
cseg at 0x0B
	jmp intt0
	
cseg at 0x1B
	jmp intt1
		
cseg at 0x30
start:
	mov SP, #0x30
	mov TMOD, #0x51
	setb ET0
	setb ET1
	setb TR0
	setb TR1
	setb EA
	mov TH1, #0xFF
	mov TL1, #0xEC
	jmp $
	
	
intt0:
	mov TH0, #0x3C
	mov TL0, #0xB0
	setb T1
	clr T1
	reti
	
intt1:
	mov TH1, #0xFF
	mov TL1, #0xEC
	inc sec
	mov a, sec
	cjne a, #60, skip
	inc min
	mov sec, #0
	skip:
	mov P1, sec
	mov P2, min
	reti
	
END
		