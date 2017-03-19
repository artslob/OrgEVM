#include <reg51.h>

unsigned char sec, min;

intt0() interrupt 1 {
	TH0 = 0x3C;
	TL0 = 0xB0;
	T1 = 1;
	T1 = 0;
}

intt1() interrupt 3 {
	TH1 = 0xFF;
	TL1 = 0xEC;
	sec++;
	if (sec == 60){
		min++;
		sec = 0;
	}
	P1 = sec;
	P2 = min;
}


main(){
	//T0 & T1 are 16-bit
	//C/T1: calc events on T1(P3.5)
	TMOD = 0x51;
	//enable T0 interrupts
	ET0 = 1;
	//enable T1 interrupts
	ET1 = 1;
	//run T0
	TR0 = 1;
	//run T1
	TR1 = 1;
	//enable all interrupts
	EA = 1;
	
	TH1 = 0xFF;
	TL1 = 0xEC;
	
	while(1);
}
