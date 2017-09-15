#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "toggleButton.h"
#include "switch.h"
#include "tankLevel.h"
#include "watering.h"

typedef struct ToggleButton
{
    char *deviceId;
    int gpio;
    char *targetDeviceId;
    void (*function)(void);

    struct ToggleButton *next;
} ToggleButtonList;
ToggleButtonList *firstToggleButton, *lastToggleButton;

void toggleTargetDeviceId(int pinNumber)
{
	printf("toggle pin number %d\n", pinNumber);
	ToggleButtonList *current = firstToggleButton;
	while (current != NULL)
	{
		if(current->gpio == pinNumber)
		{
			printf("toggleDeficeId %s\n", current->deviceId);

    			if (strncmp("tankLevel", current->targetDeviceId, strlen("tankLevel")) == 0)
    			{
        			triggerTankLevel(current->targetDeviceId);
    			}
    			else if (strncmp("switch", current->targetDeviceId, strlen("switch")) == 0)
    			{
        			toggleSwitch(current->targetDeviceId);
    			}
    			else if (strncmp("watering", current->targetDeviceId, strlen("watering")) == 0)
    			{
        			triggerWatering(current->targetDeviceId);
    			}
			return;
		}
		current = current->next;
	}
}

void addToggleButton(char *toggleButtonId, int gpio, char *targetDeviceId)
{
    ToggleButtonList *newDevice = malloc(sizeof(ToggleButtonList));
    newDevice->deviceId = malloc(strlen(toggleButtonId) * sizeof(char));
    strcpy(newDevice->deviceId, toggleButtonId);

    newDevice->targetDeviceId = malloc(strlen(targetDeviceId) * sizeof(char));
    strcpy(newDevice->targetDeviceId, targetDeviceId);

    newDevice->gpio = gpio;

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
    pullUpDnControl(newDevice->gpio, PUD_UP);
    
    wiringPiISR(newDevice->gpio, INT_EDGE_RISING, &toggleTargetDeviceId);
}
