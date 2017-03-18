dseg at 0x08
	sec: ds 1
	min: ds 1
	count: ds 1

cseg at 0x00
	mov SP, #0x30
	jmp start

cseg at 0x0B
	jmp intt0
	
	
cseg at 0x24
start:
	mov TMOD, #0x01 ;16 битный таймер
	setb ET0        ;вкл. прерывания от таймера 0
	setb TR0        ;запустить таймер 0
	setb EA         ;включить прерывания
	jmp $
	

intt0:
	mov TH0, #0x3C
	mov TL0, #0xB0
	inc count
	mov a, count
	cjne a, #20, skip1
	inc sec
	mov count, #0x00
	
	skip1:
	mov a, sec
	cjne a, #60, skip2
	inc min
	mov sec, #0x00
	
	skip2:
	mov P1, sec
	mov P2, min
	
	/*
	if (sec == 60){
		min++;
		sec = 0;
	}
	P1 = sec;
	P2 = min;
	*/
	reti
	
END
