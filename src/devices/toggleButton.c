#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>

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

	logDebug("[ToggleButton] Toggle pin number %d with value %d (target deviced not triggered yet)", pinNumber, digitalRead(pinNumber));

	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		if (current->gpio == pinNumber)
		{
			logDebug("[ToggleButton] Found toggleDeviceId %s", current->deviceId);
			time_t rawtime;
			time(&rawtime);
			long differenceInSec = (long)rawtime - current->lastTriggerTime;
			
			if (differenceInSec < 1)
			{
				logDebug("[ToggleButton] Difference from last toggle is too short, skip triggering: %ld", differenceInSec);	
				return;
			}

			logDebug("[ToggleButton] Difference from last toggle is valid: %ld", differenceInSec);

			current->lastTriggerTime = (long)rawtime;

			sendCommand(current->targetDeviceId);
			return;
		}
		current = current->next;
	}
}

int setNightWithness(char *targetDeviceId)
{
	int indexOfColon = strcspn(targetDeviceId, ":");
	logDebug("[ToggleButton] index of colon %d", indexOfColon);
	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		if (strncmp(current->targetDeviceId, targetDeviceId, indexOfColon) == 0)
		{
			logDebug("[ToggleButton] led should be notified after this step %s and state ", current->targetDeviceId);
			if (targetDeviceId[indexOfColon + 1] == '0')
			{
				digitalWrite(current->ledGpio, HIGH);
			}
			else
			{
				digitalWrite(current->ledGpio, LOW);
			}
			return 1;
		}

		current = current->next;
	}

	return 0;
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
	logInfo("[ToggleButton] Gpio for button: %d", newDevice->gpio);
	wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &toggleTargetDeviceId);
}