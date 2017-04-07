#include <reg51.h>
#include <stdio.h> //библиотека ввода-вывода C51

char i, s, min, aa[6];
float m, numb;
int x;

void what(unsigned char digit);
void get_char();

Seria() interrupt 4 {
	s = getkey();
	//aa[i++] = s;
	what(s);
}

main() {
	SCON = 0x50; //sm=1, ren=1
	TMOD = 0x20; //таймер 1 - режим 2
	TH1 = 0xfd; //константа автозагрузки - частота 9600 бод
	i = 0;
	ES = 1; //маска прерывания
	EA = 1;
	TR1 = 1;
	
	while(1) {
		while(i != 5);
		get_char();
		aa[5] = 0;
		EA = 0;
		TI = 1; //начальная установка готовности передачи
		printf("aa = %s\n", aa); // форматированный вывод в USART
		printf("x = %f\n", numb);
		i = 0;
		EA = 1; //маска прерывания
	}
}

void what(unsigned char digit) {
	if (digit == ',') {
		m = 1;
		aa[i++] = digit;
	}
	else if (digit == '-') {
		min = 1;
		aa[i++] = digit;
	}
	else {
		aa[i++] = digit;
		numb = numb * 10 + (digit & 0x0F);
		if (m)
			m *= 10;
	}
}

void get_char() {
	if (m)
		numb = numb / m;
	if (min)
		numb *= -1;
}