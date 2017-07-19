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

int hasSoilEnoughWater(void)
{
	//TODO: when I have a moisture sensor this method should return the actual state of the soil
	return 0;
}	

volatile int isWateringSystemOverrided = 0;
struct tm* wateringSystemOverridedOn;

void timerCallbackWatering(struct tm * timeinfo)
{		
		if(isWateringSystemOverrided)
		{
			//TODO: take over control  after 12 h
		}
		else if(timeinfo ->tm_sec >= 15 && timeinfo -> tm_sec <=45 && !hasSoilEnoughWater())
		{			
			digitalWrite(commandPinTankOutputEv, HIGH);					
		}
		else
		{
			digitalWrite(commandPinTankOutputEv, LOW);			
		}	
		
		if(digitalRead(commandPinTankOutputEv))
		{
			digitalWrite(ledPinTankOutputEvOperation, state);
		}
		else
		{
			digitalWrite(ledPinTankOutputEvOperation, LOW);
		}
}

void triggerWatering(void)
{	
	time_t rawtime;
	time ( &rawtime );
	wateringSystemOverridedOn = localtime ( &rawtime );
	isWateringSystemOverrided = 1;
	
	int currentState = digitalRead(commandPinTankOutputEv);
	digitalWrite(commandPinTankOutputEv, !currentState);
	digitalWrite(ledPinTankOutputEvOperation, !currentState);
}

