#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>

#include "../main.h"
#include "devices.h"
regex_t toggleButtonRegex;

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

			sendMessage(COMMAND, current->deviceId, NULL);
			return;
		}
		current = current->next;
	}
}

int setNightWithness(char *targetDeviceId)
{
	if (regexec(&toggleButtonRegex, targetDeviceId, 0, NULL, 0))
	{
		return 0;
	}
    int indexOfColon = strcspn(targetDeviceId, ":");
	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		if (strncmp(current->targetDeviceId, targetDeviceId, indexOfColon) == 0)
		{

			// printf("led should be notified after this step %s and state %s\n", current->targetDeviceId, getDeviceState(current->targetDeviceId));
			 if (targetDeviceId[indexOfColon + 1] == '1')
			{
				digitalWrite(current->ledGpio, HIGH);
			}
			else
			{
				digitalWrite(current->ledGpio, LOW);
			}
		
		}
		
		current = current->next;
	}
	
	return 1;
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

void initToggleButton(){	
	if (regcomp(&toggleButtonRegex, "swithc[0-9]+:[01]", 0))
	{
		logError("[ToggleButton] Regex pattern could not be compiled");
	}
}
