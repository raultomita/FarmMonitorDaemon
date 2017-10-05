#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../main.h"
#include "devices.h"

typedef struct ToggleButton
{
	char *deviceId;
	int gpio;
	int ledGpio;
	char *targetDeviceId;
	time_t lastTriggerTime;

	struct ToggleButton *next;
} ToggleButtonList;
ToggleButtonList *firstToggleButton, *lastToggleButton;

void toggleTargetDeviceId(int pinNumber)
{
	if (digitalRead(pinNumber) == LOW)
		return;

	printf("toggle pin number %d with value %d\n", pinNumber, digitalRead(pinNumber));

	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		if (current->gpio == pinNumber)
		{
			time_t rawtime;
			time(&rawtime);
			double differenceInSec = difftime(rawtime, current->lastTriggerTime);
			printf("difference is %f\n", differenceInSec);

			if (differenceInSec <= 1)
			{
				return;
			}

			current->lastTriggerTime = rawtime;

			printf("toggleDeficeId %s\n", current->deviceId);

			triggerDevice(current->targetDeviceId);
			return;
		}
		current = current->next;
	}
}

void setNightWithness(char *targetDeviceId){
	
	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		if (strcmp(current->targetDeviceId, targetDeviceId) == 0)
		{
			if(strcmp(getDeviceState(current->targetDeviceId), "0") == 0){
			digitalWrite(current->ledGpio, HIGH);
			}
			else{digitalWrite(current->ledGpio, LOW);}
			
		}
		current = current->next;
	}
}

void addToggleButton(char *toggleButtonId, int gpio, int ledGpio, char *targetDeviceId)
{
	ToggleButtonList *newDevice = malloc(sizeof(ToggleButtonList));
	newDevice->deviceId = malloc(strlen(toggleButtonId) * sizeof(char));
	strcpy(newDevice->deviceId, toggleButtonId);

	newDevice->targetDeviceId = malloc(strlen(targetDeviceId) * sizeof(char));
	strcpy(newDevice->targetDeviceId, targetDeviceId);

	newDevice->gpio = gpio;
	newDevice->ledGpio = ledGpio;

	if (firstToggleButton == NULL)
	{
		firstToggleButton = newDevice;
		lastToggleButton = newDevice;
	}
	else
	{
		lastToggleButton->next = newDevice;
		lastToggleButton = newDevice;
	}

	pinMode(newDevice->gpio, INPUT);
	pinMode(newDevice->ledGpio, OUTPUT);
	pullUpDnControl(newDevice->gpio, PUD_UP);
	printf("Gpio for button: %d\n", newDevice->gpio);
	wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &toggleTargetDeviceId);
}
