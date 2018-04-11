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
	int targetStatus;
	char *targetDeviceId;
	struct DistanceSensor *next;

} DistanceSensorList;
DistanceSensorList *firstDistanceSensor, *lastDistanceSensor;

void distanceChanged(int pinNumber)
{
	
	

	DistanceSensorList *current = firstDistanceSensor;
	while (current != NULL)
	{
		if (current->gpio == pinNumber && digitalRead(pinNumber) == LOW && current->targetStatus == HIGH)
		{			
			current->targetStatus = LOW;
			logDebug("[DistanceSensor] Found distanceSensor %s %d", current->deviceId, digitalRead(pinNumber));
			triggerDevice(current->targetDeviceId);
			return;
		}
		current = current->next;
	}
}

int setTargetStatus(char *targetDeviceId)
{
	int indexOfColon = strcspn(targetDeviceId, ":");
	logDebug("[DistanceSensor] index of colon %d", indexOfColon);
	DistanceSensorList *current = firstDistanceSensor;
	while (current != NULL)
	{
		if (strncmp(current->targetDeviceId, targetDeviceId, indexOfColon) == 0)
		{			
			if (targetDeviceId[indexOfColon + 1] == '0')
			{
				current->targetStatus = HIGH;
			}
			else
			{
				current->targetStatus = LOW;
			}
			logDebug("[DistanceSensor] State is saved after this step %s and state %d", current->targetDeviceId, current->targetStatus);
			return 1;
		}

		current = current->next;
	}

	return 0;
}

void addDistanceSensor(char *distanceSensorId, int gpio, char *targetDeviceId)
{
	DistanceSensorList *newDevice = malloc(sizeof(DistanceSensorList));
	newDevice->deviceId = malloc(strlen(distanceSensorId) * sizeof(char));
	strcpy(newDevice->deviceId, distanceSensorId);

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
	newDevice->targetStatus = LOW;
	pinMode(newDevice->gpio, INPUT);
	pullUpDnControl(newDevice->gpio, PUD_UP);	
	wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &distanceChanged);

	logInfo("[DistanceSensor] Gpio for sensor: %d", newDevice->gpio);
}
