#include <reg51.h>

char w, x, i, digit, mas[7], min;
float numb, m;
char code scancode[] = { 0xDE, 0xD7, 0xEE, 0xB7, 0x77, 0x7B, 0xBE }; //-1,234 esc

void Delay (int t) {
	while(t--);
}

char what(void);

scan( ) interrupt 2 {
	Delay(100);
	w = scancode[i];
	what();
	Delay(100);
}

main () {
	x = 0xFE;
	numb = 0;
	i = 0;
	m = 0;
	//прерывание 1-0 на INT1(P3.3), вкл external 1, вкл прерывания 
	IT1 = EX1 = EA = 1;
	
	while (digit != 'e')
		Delay(100);
	
	while(1);
}

char what(void) {
	switch (w) {
		case 0xE7: digit = '0'; break;
		case 0xD7: digit = '1'; break;
		case 0xB7: digit = '2'; break;
		case 0x77: digit = '3'; break;
		case 0x7B: digit = '4'; break;
		case 0xBB: digit = '5'; break;
		case 0xDB: digit = '6'; break;
		case 0xEB: digit = '7'; break;
		case 0xED: digit = '8'; break;
		case 0xDD: digit = '9'; break;
		case 0xEE: digit = ','; break;
		case 0xDE: digit = '-'; break;
		case 0xBE: digit = 'e'; break;
		default: digit=0xff;
	}
	if (digit == ',') {
		m = 1;
		mas[i++] = digit;
	}
	else if (digit == '-') {
		min = 1;
		mas[i++] = digit;
	}
	else if (digit == 'e') {
		if (m)
			numb = numb / m;
		if (min)
			numb *= -1;
		i = 0;
		m = 0;
	}
	else {
		mas[i++] = digit;
		numb = numb * 10 + (digit & 0xf);
		if (m)
			m *= 10;
	}
	return digit; 
}