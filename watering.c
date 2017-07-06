#include <wiringPi.h>
#include "display.h"
#include "watering.h"
#include "tankLevel.h"
#include "pins.h"

void initializeWateringSchedule(void)
{
	pinMode(ledPinTankOutputEv, OUTPUT);
	digitalWrite(ledPinTankOutputEv, LOW);
}


void timerCallbackWatering(void)
{
	if(getTankLevel() == 100)
	{
		//printf("Turn on\n");
		digitalWrite(ledPinTankOutputEv, HIGH);
	}
	else
	{
		//printf("Turn off\n");
		digitalWrite(ledPinTankOutputEv, LOW);
	}
}
