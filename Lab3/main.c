#include <reg51.h>
#include <stdio.h>

char d[8];
char count;

intt0() interrupt 0 {
	TR0=0;
	TR1=0;
	if (TL0 <= 100)
		d[count++] = '1';
	else
		d[count++] = '0';
	
	if (count == 8)
		count = 0;
	
	TL1 = TH1 = TL0 = TH0 = 0;
	TR0 = 1;
	TR1 = 1;
}

main() {
	IT0 = EX0= 1; //external interrupt on INT0(P3.2) по 1-0 включить
	TMOD = 0x19; //t1 = 16bit; t0 = 16bit & gate1
	TL1 = TH1 = TL0 = TH0 = 0;
	TR0 = TR1 = 1;
	EA = 1;
	while(1) {}
}