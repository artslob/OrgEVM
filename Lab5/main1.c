#include <reg515.h>

main() {
	char i;
	double x;
	while(1)
	for(i = 0; i < 100; i++) {
		DAPR = 0xD0; //запуск преобразования c опорным напряжением 4в
		while(BSY); //ожидание завершения преобразования
		//x = (double) (4.0 * ADDAT / 0x100);
		P2 = ADDAT; //вывод результата }
	}
}