dseg at 0x10
	count: ds 1
	d: ds 8

cseg at 0x00
	jmp start
	
cseg at 0x03
	jmp intt0
	
intt0:
	anl TCON, #0xAF //TR0=0; TR1=0;
	mov R0, count
	mov a, #d
	add a, R0
	mov R0, a
	inc count
	clr c
	mov a, TL0
	subb a, #100
	jnc tl0_gr_or_eq 
	;if (TL0 < 100) d[count++] = '1';
	mov @R0, #0x31
	jmp check_for_eq_8
	
	tl0_gr_or_eq:
	;else d[count++] = '0';
	mov @R0, #0x30
	
	check_for_eq_8:
	mov a, count
	cjne a, #8, skip
	mov count, #0 ;if (count == 8) count = 0;
	
	skip:
	;TL1 = TH1 = TL0 = TH0 = 0;
	mov TL1, #0
	mov TH1, #0
	mov TL0, #0
	mov TH0, #0
	orl TCON, #0x50 ;TR0 = 1; TR1 = 1;
	reti
	
	
start:
	mov SP, #0x30
	mov TMOD, #0x19 //t1 = 16bit; t0 = 16bit & gate1
	setb EX0  		//interrupt on INT0(P3.2) по 1-0
	orl TCON, #0x51 //TR0 = TR1 = 1; IT0 = 1;
	setb EA
	jmp $
	END