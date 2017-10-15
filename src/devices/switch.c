#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#include "../main.h"
#include "devices.h"

const char *switchJsonFormat =
	"{ \"id\": \"%s\", \"type\": \"switch\", \"display\":\"%s\", \"location\":\"%s\", \"timeStamp\": \"%s\", \"state\": \"%d\" }";
regex_t switchRegex;

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

	char *switchState = (char *)malloc(sizeof(switchItem->deviceId) + 2 * sizeof(char));
	sprintf(switchState, "%s:%d", switchItem->deviceId, digitalRead(switchItem->gpio));

	sendMessage(NOTIFICATION, switchItem->deviceId, json);
	sendMessage(COMMAND, switchState, NULL);
	printf("%s notification sent to redis\n", switchItem->deviceId);
}

int toggleSwitch(char *switchId)
{
	logInfo("[Switch] Enter switch for %s", switchId);
	int ret = regexec(&switchRegex, switchId, 0, NULL, 0);
	logInfo("[Switch] regExt return %d", ret);
	if (regexec(&switchRegex, switchId, 0, NULL, 0))
	{
		return 0;
	}

	SwitchList *current = firstSwitch;

	while (current != NULL)
	{
		logInfo("[Switch] internal deviceId %s whereas parameter switchId is %s", current->deviceId, switchId);
		if (strcmp(current->deviceId, switchId) == 0)
		{
			digitalWrite(current->gpio, !digitalRead(current->gpio));
			sendSwitchNotification(current);
		}
		logInfo("[Switch] move to next Switch");
		current = current->next;
		logInfo("[Switch] moved to next Switch %d", current != NULL);
	}

	return 1;
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

	newDevice->next = NULL;
	pinMode(gpio, OUTPUT);
	digitalWrite(gpio, LOW);
	sendSwitchNotification(newDevice);
}

void initSwitch(void)
{
	if (regcomp(&switchRegex, "^switch[0-9*]$", 0))
	{
		logError("[Switch] Regex pattern could not be compiled");
	}
}
