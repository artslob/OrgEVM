cseg at 0x00
	mov r1, #2
	mov r2, #12
	mov r3, #0xFF
	ljmp 0x20
	
cseg at 0x20
start:
	mov a, r2       // ACC = 12
	dec r2          // r2 = 11
	mov a, 2        // ACC = 11, p = 1 (psw = 0x01)
	dec @r1         // r2 = 10
	mov a, 2        // ACC = 10 , psw = 0x00
	dec 2           // r2 = 9
	mov a, 2        // ACC = 9
	
	mov a, 3        // ACC = 0xFF
	add a, #1       // ACC = 0, psw = 0xC0
	anl c, /ACC.0   // nothings changes
	anl c, ACC.0    // psw = 0x40
	
	jz start
	END