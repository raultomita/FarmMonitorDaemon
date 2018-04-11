#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>

#include "../main.h"
#include "devices.h"

typedef struct DistanceSensor
{
	char *deviceId;
	int gpio;
	char *targetDeviceId;
	struct DistanceSensor *next;

} DistanceSensorList;
DistanceSensorList *firstDistanceSensor, *lastDistanceSensor;

void distanceChanged(int pinNumber)
{
	
	logDebug("[DistanceSensor] Distance pin number %d with value %d (target deviced not triggered yet)", pinNumber, digitalRead(pinNumber));

	DistanceSensorList *current = firstDistanceSensor;
	while (current != NULL)
	{
		if (current->gpio == pinNumber)
		{
			logDebug("[DistanceSensor] Found distanceSensor %s", current->deviceId);
			
			//char *switchState = (char*)malloc((sizeof(current->deviceId)+2) * sizeof(char));
    		//sprintf(switchState, "%s:%d", current->deviceId, digitalRead(current->gpio));
			//triggerDevice(switchState);
			return;
		}
		current = current->next;
	}
}


void addDistanceSensor(char *distanceSensorId, int gpio, char *targetDeviceId)
{
	DistanceSensorList *newDevice = malloc(sizeof(DistanceSensorList));
	newDevice->deviceId = malloc(strlen(DistanceSensorId) * sizeof(char));
	strcpy(newDevice->deviceId, DistanceSensorId);

	newDevice->targetDeviceId = malloc(strlen(targetDeviceId) * sizeof(char));
	strcpy(newDevice->targetDeviceId, targetDeviceId);

	newDevice->gpio = gpio;

	if (firstDistanceSensor == NULL)
	{
		firstDistanceSensor = newDevice;
		lastDistanceSensor = newDevice;
	}
	else
	{
		lastDistanceSensor->next = newDevice;
		lastDistanceSensor = newDevice;
	}

	newDevice->next = NULL;
	
	pinMode(newDevice->gpio, INPUT);
	pullUpDnControl(newDevice->gpio, PUD_UP);	
	wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &distanceChanged);

	logInfo("[DistanceSensor] Gpio for sensor: %d", newDevice->gpio);
}
