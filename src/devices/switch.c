#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../main.h"
#include "devices.h"

const char *switchJsonFormat =
	"{ \"id\": \"%s\", \"type\": \"switch\", \"display\":\"%s\", \"location\":\"%s\", \"timeStamp\": \"%s\", \"state\": \"%d\" }";

typedef struct Switch
{
	char *deviceId;
	char *display;
	char *location;
	int gpio;
	struct Switch *next;
} SwitchList;
SwitchList *firstSwitch, *lastSwitch;

void sendSwitchNotification(SwitchList *switchItem)
{
	printf("%s sending\n", switchItem->deviceId);

	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));

	char *json = (char *)malloc((strlen(switchJsonFormat) + strlen(timeString) + strlen(switchItem->deviceId) +
								 strlen(switchItem->display) + strlen(switchItem->location)) *
								sizeof(char));

	sprintf(json,
			switchJsonFormat,
			switchItem->deviceId,
			switchItem->display,
			switchItem->location,
			timeString,
			digitalRead(switchItem->gpio));

	sendMessage(NOTIFICATION, switchItem->deviceId, json);
	printf("%s notification sent to redis\n", switchItem->deviceId);
}

int toggleSwitch(char *switchId)
{
	SwitchList *current = firstSwitch;

	while (current != NULL)
	{
		if (strcmp(current->deviceId, switchId) == 0)
		{
			digitalWrite(current->gpio, !digitalRead(current->gpio));
			sendSwitchNotification(current);
			return 1;
		}

		current = current->next;
	}

	return 0;
}

//Public APIs
void addSwitch(char *switchId, char *display, char *location, int gpio)
{
	SwitchList *newDevice = malloc(sizeof(SwitchList));
	newDevice->deviceId = malloc(strlen(switchId) * sizeof(char));
	strcpy(newDevice->deviceId, switchId);

	newDevice->display = malloc(strlen(display) * sizeof(char));
	strcpy(newDevice->display, display);

	newDevice->location = malloc(strlen(location) * sizeof(char));
	strcpy(newDevice->location, location);

	newDevice->gpio = gpio;

	if (firstSwitch == NULL)
	{
		firstSwitch = newDevice;
		lastSwitch = newDevice;
	}
	else
	{
		lastSwitch->next = newDevice;
		lastSwitch = newDevice;
	}

	pinMode(gpio, OUTPUT);
	digitalWrite(gpio, LOW);
	sendSwitchNotification(newDevice);
}
