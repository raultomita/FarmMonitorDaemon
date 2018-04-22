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
	int invertState;
	long lastTriggerTime;
	struct DistanceSensor *next;

} DistanceSensorList;
DistanceSensorList *firstDistanceSensor, *lastDistanceSensor;

void distanceChanged(int pinNumber)
{
	logDebug("[DistanceSensor] Found distance %d", digitalRead(pinNumber));

	DistanceSensorList *current = firstDistanceSensor;
	while (current != NULL)
	{
		if (current->gpio == pinNumber)
		{		
		    if (current->invertState == 1){
				time_t rawtime;
			    time(&rawtime);
			    long differenceInSec = (long)rawtime - current->lastTriggerTime;
			
				if(differenceInSec > 1 && digitalRead(pinNumber) == LOW){
					logDebug("[DistanceSensor] Trigger device %s", current->targetDeviceId);
					current->lastTriggerTime = (long)rawtime;
					triggerDevice(current->targetDeviceId);

				}

			}
			else if (digitalRead(pinNumber) == LOW && current->targetStatus == HIGH)
			{
				current->targetStatus = LOW;
				logDebug("[DistanceSensor] Turn OFF device %s", current->targetDeviceId);
				triggerDevice(current->targetDeviceId);
			}
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
				current->targetStatus = LOW;
			}
			else
			{
				current->targetStatus = HIGH;
			}
			logDebug("[DistanceSensor] State is saved after this step %s and state %d", current->targetDeviceId, current->targetStatus);
			return 1;
		}

		current = current->next;
	}

	return 0;
}

void addDistanceSensor(char *distanceSensorId, int gpio, char *targetDeviceId, int invertState)
{
	DistanceSensorList *newDevice = malloc(sizeof(DistanceSensorList));
	newDevice->deviceId = malloc(strlen(distanceSensorId) * sizeof(char));
	strcpy(newDevice->deviceId, distanceSensorId);

	newDevice->targetDeviceId = malloc(strlen(targetDeviceId) * sizeof(char));
	strcpy(newDevice->targetDeviceId, targetDeviceId);

	newDevice->gpio = gpio;
	newDevice->invertState = invertState;

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

	newDevice->lastTriggerTime = 0;
	newDevice->next = NULL;
	newDevice->targetStatus = LOW;
	pinMode(newDevice->gpio, INPUT);
	pullUpDnControl(newDevice->gpio, PUD_UP);	
	wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &distanceChanged);

	logInfo("[DistanceSensor] Gpio for sensor: %d", newDevice->gpio);
}
