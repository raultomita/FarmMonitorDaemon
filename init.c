#include <wiringPi.h>
#include <stdio.h>
#include "tankLevel.h"

int main(void)
{
	wiringPiSetupGpio();
	initializeTankLevel();
	printf("Configuration complete \n");

	while(1){
		printf("I'm alive and current tank level: %d\n", getTankLevel());
		delay(10000);
	}
        return 0;
}
