#include <wiringPi.h>
#include <time.h>
#include "display.h"
#include "watering.h"
#include "tankLevel.h"
#include "pins.h"

void initializeWateringSchedule(void)
{
	pinMode(commandPinTankOutputEv, OUTPUT);
	pinMode(ledPinTankOutputEvOperation, OUTPUT);
	digitalWrite(commandPinTankOutputEv, LOW);
}

int hasEnoughWatterIsSoil(void)
{
	//TODO: when I have a moisture sensor this method should return the actual state of the soil
	return 0;
}	

void timerCallbackWatering(struct tm * timeinfo)
{	
	if(timeinfo ->tm_sec >= 15 && timeinfo -> tm_sec <=45 && !hasEnoughWatterIsSoil())
	{		
		digitalWrite(commandPinTankOutputEv, HIGH);		
		digitalWrite(ledPinTankOutputEvOperation, state);
	}
	else
	{
		digitalWrite(commandPinTankOutputEv, LOW);
		digitalWrite(ledPinTankOutputEvOperation, LOW);
	}
}
