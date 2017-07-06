#include <wiringPi.h>
#include <stdio.h>
#include "tankLevel.h"
#include "watering.h"
#include "pins.h"

int ledPinTankFull = 23;
int ledPinTankEmpty = 18;

int ledPinTankInputEv = 17;
int ledPinTankOutputEv = 24;

int btnPinDrain = 22;
int btnPinFill=27;

int main(void)
{
	wiringPiSetupGpio();
	initializeTankLevel();
	initializeWateringSchedule();
	printf("Configuration complete \n");

	while(1)
	{
		delay(1000);
		timerCallbackWatering();
		timerCallbackTankLevel();		
	}
    return 0;
}
