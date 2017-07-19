#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "display.h"
#include "watering.h"
#include "tankLevel.h"
#include "notification.h"
#include "pins.h"

const char * wateringStateJsonFormat = 
 "{ id: 'watering', type: 'watering', timeStamp:'TBD', state: '%d' }";

void sendWateringNotification(void)
{
	char *json=(char*)malloc(strlen(wateringStateJsonFormat) * sizeof(char));
	sprintf(json, wateringStateJsonFormat,
		digitalRead(commandPinTankOutputEv));
	saveAndNotify("watering", json);
}

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
	int wateringState =digitalRead(commandPinTankOutputEv);
	if(isWateringSystemOverrided)
	{
		//TODO: take over control  after 12 h
	}
	else if(timeinfo ->tm_sec >= 15 && timeinfo -> tm_sec <=45 && !hasSoilEnoughWater())
	{			
		wateringState = HIGH;					
	}
	else
	{
		wateringState = LOW;			
	}	
	if(wateringState != digitalRead(commandPinTankOutputEv)){
		digitalWrite(commandPinTankOutputEv, wateringState);
		sendWateringNotification();
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
	sendWateringNotification();
}


