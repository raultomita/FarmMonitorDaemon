#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "notification.h"

const char *switchJsonFormat =
	"{ \"id\": \"switch%d\", \"type\": \"switch\", \"timeStamp\": \"%s\", \"state\": \"%d\" }";

struct SwitchItem
{
	char *deviceId;
	char *display;
	char *location;
	int gpio;
	SwitchItem *next;
} * firstSwitch, *lastSwitch;

void sendSwitchNotification(SwitchItem *switchItem)
{
	printf("Switch %d sending\n", number);
	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));

	char *json = (char *)malloc((strlen(switchJsonFormat) + strlen(timeString) + 1) * sizeof(char));
	sprintf(json,
			switchJsonFormat,
			number,
			timeString,
			digitalRead(switchPins[number]));
	printf("Switch %d notification sent\n", number);
	saveAndNotify("switch", json);
	printf("Switch %d notification sent to redis\n", number);
}

void toggleSwitch(char *switchId)
{
	SwitchItem *current = *firstSwitch;

	while (current != null)
	{
		if (strcmp(current->deviceId, switchId) == 0)
		{
			digitalWrite(current->gpio, !digitalRead(curren->gpio));
			sendSwitchNotification(current);
			return;
		}

		current = current->next;
	}
}

//Public APIs
void addSwitch(char *switchId, char *display, char *location, int gpio)
{
	SwitchItem *newDevice = malloc(sizeof(SwitchItem));
	newDevice->deviceId = malloc(strlen(swithcId) * sizeof(char));
	strcpy(newDevice->deviceId, switchId);
	newDevice->gpio = gpio;

	if (firstSwitch == NULL)
	{
		firstSwitch = newDevice;
		lastSwitch = newDevice;
	}
	else
	{
		lastSwitch->next = current;
		lastSwitch = current;
	}

	pinMode(gpio, OUTPUT);
	digitalWrite(gpio, LOW);
	sendSwitchNotification(current);
}
