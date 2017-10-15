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
	struct ToggleButton *next;
	long lastTriggerTime;

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
			long differenceInSec = (long)rawtime - current->lastTriggerTime;
			printf("difference is %ld\n", differenceInSec);

			if (differenceInSec < 1)
			{
				return;
			}

			current->lastTriggerTime = (long)rawtime;

			printf("toggleDeficeId %s\n", current->deviceId);

			triggerDevice(current->targetDeviceId);
			return;
		}
		current = current->next;
	}
}

void setNightWithness(char *targetDeviceId)
{

	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		printf("Enter again %d\n", current != NULL);
		if (strcmp(current->targetDeviceId, targetDeviceId) == 0)
		{
			requestDeviceState(current->targetDeviceId);
			// printf("led should be notified after this step %s and state %s\n", current->targetDeviceId, getDeviceState(current->targetDeviceId));
			// if (strcmp(getDeviceState(current->targetDeviceId), "0") == 0)
			// {
			// 	digitalWrite(current->ledGpio, HIGH);
			// }
			// else
			// {
			// 	digitalWrite(current->ledGpio, LOW);
			// }
			printf("led should be notified already\n");
		}
		printf("finish night withness sensor %s\n", current->deviceId);
		printf("next toggle %d\n", current->next != NULL);
		current = current->next;
	}
	printf("finish night withness\n");
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

	newDevice->next = NULL;
	pinMode(newDevice->gpio, INPUT);
	pinMode(newDevice->ledGpio, OUTPUT);
	pullUpDnControl(newDevice->gpio, PUD_UP);
	printf("Gpio for button: %d\n", newDevice->gpio);
	wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &toggleTargetDeviceId);
}
