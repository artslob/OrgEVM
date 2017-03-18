#include <reg51.h>


main(){
	while(1);
}




/*  version from nazariev(not working):
 
unsigned char f;

intt0() interrupt 1 {
	P1 = TH1;
	P2 = TL1;
	TH1 = 0;
	TL1 = 0;
	TH0 = -1;
	TL0 = -1;
}

main(){
	TMOD = 21; 
	// timer 0 16-bit
	// C/T0 = 1, GATE0 = 1
	// timer 1 16-bit
				
	TH0 = -1;
	TL0 = -1; //timer0 = -1
	
	TR1 = 1;  //tun timer 1
	ET0 = 1;  //timer 0 interrupt on
	EA = 1;   //all interrupts on
	TR0 = 1;  //tun timer 0
	IT0 = 1;  //interrupts on 1-0 int0(P3.2)
	while(1) { }
}

*/