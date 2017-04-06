#include <reg515.h>

int mm;
unsigned int T;
unsigned char max, min;
float fmax;

unsigned char adc(void);

main() {
	max = 0;
	min = 0x70;
	while(INT0) {
		adc();
		if (P2 > max) max = P2;
		if (P2 < min) min = P2;
	}
	mm = (max + min) / 2;
	TMOD = 1;
	TH0 = TL0 = 0;
	TR0 = 0;
	while(adc() >= mm);
	while(adc() <= mm);
	TR0 = 1;
	while(adc() >= mm);
	TR0 = 0;
	T = (TH0 << 8) + TL0;
	fmax = max * 4.0 / 0x100;
	while(1);
}

unsigned char adc(void) {
	DAPR = 0xD0;
	while(BSY); //ожидание завершения преобразования
	P2 = ADDAT;
	return P2;
}
	
	
	
/*main() {
	char i;
	while(1)
		for (i = 0; i < 100; i++) {
			DAPR = 0;
			while(BSY);
			P2 = ADDAT;
		}
}*/
