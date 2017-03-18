#include <reg51.h>

unsigned char sec, min;
unsigned char count = 0;

intt0() interrupt 1 {
	TH0 = 0x3C;
	TL0 = 0xB0;
	count++;
	if (count == 20){
		sec++;
		count = 0;
	}
	if (sec == 60){
		min++;
		sec = 0;
	}
	P1 = sec;
	P2 = min;
}

main(){
	TMOD = 1; //16 битный таймер
	ET0 = 1;  //вкл. прерывания от таймера 0
	TR0 = 1;  //запустить таймер 0
	EA = 1;   //включить прерывания
	while(1);
}