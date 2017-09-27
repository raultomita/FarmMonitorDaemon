#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "devices.h"

const char *tankLevelJsonFormat =
	"{ \"id\": \"%s\", \"type\": \"tankLevel\", \"display\":\"%s\", \"location\":\"%s\", \"timeStamp\": \"%s\", \"level\": \"%d\", \"state\": \"%d\" }";

typedef struct TankLevel
{
	char *deviceId;
	char *display;
	char *location;
	int commandGpio;
	int notifyGpio;
	int levelGpio;
	int level;

	struct TankLevel *next;
} TankLevelList;
TankLevelList *firstTankLevel, *lastTankLevel;

void sendTankLevelNotification(TankLevelList *tankLevel)
{
	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));

	printf("%s\n", timeString);

	char *json = (char *)malloc((strlen(tankLevelJsonFormat) + strlen(timeString) + strlen(tankLevel->deviceId) +
								 strlen(tankLevel->display) + strlen(tankLevel->location)) * sizeof(char));
	sprintf(json,
			tankLevelJsonFormat,			
			tankLevel->deviceId,
			tankLevel->display,
			tankLevel->location,
			timeString,
			tankLevel->level,
			digitalRead(tankLevel->commandGpio));

	sendMessage(NOTIFICATION, tankLevel->deviceId, json);
}

void displayTankLevel(TankLevelList *tankLevel)
{
	if (tankLevel->level < 10)
	{
		digitalWrite(tankLevel->levelGpio, LOW);
	}
	else
	{
		digitalWrite(tankLevel->levelGpio, HIGH);
	}

	printf("Level is %d\n", tankLevel->level);
}

void triggerElectroValve(TankLevelList *tankLevel)
{
	if (tankLevel->level <= 3)
	{
		digitalWrite(tankLevel->commandGpio, HIGH);
	}
	else if (tankLevel->level == 10)
	{
		digitalWrite(tankLevel->commandGpio, LOW);
	}
}

//Public APIs
void addTankLevel(char *tankLevelId, char *display, char *location, int commandGpio, int notifyGpio, int levelGpio)
{
	TankLevelList *newDevice = malloc(sizeof(TankLevelList));
	newDevice->deviceId = malloc(strlen(tankLevelId) * sizeof(char));
	strcpy(newDevice->deviceId, tankLevelId);

	newDevice->display = malloc(strlen(display) * sizeof(char));
	strcpy(newDevice->display, display);

	newDevice->location = malloc(strlen(location) * sizeof(char));
	strcpy(newDevice->location, location);

	newDevice->commandGpio = commandGpio;
	newDevice->notifyGpio = notifyGpio;
	newDevice->levelGpio = levelGpio;
	newDevice->level = 0;

	if (firstTankLevel == NULL)
	{
		firstTankLevel = newDevice;
		lastTankLevel = newDevice;
	}
	else
	{
		lastTankLevel->next = newDevice;
		lastTankLevel = newDevice;
	}

	pinMode(newDevice->commandGpio, OUTPUT);
	pinMode(newDevice->notifyGpio, OUTPUT);
	pinMode(newDevice->levelGpio, OUTPUT);
	
	//Query tank lavel from sensor
	displayTankLevel(newDevice);
	triggerElectroValve(newDevice);
	digitalWrite(newDevice->notifyGpio, LOW);
	sendTankLevelNotification(newDevice);

}

int triggerTankLevel(char *deviceId)
{
	TankLevelList *current = firstTankLevel;

	while (current != NULL)
	{
		if (strcmp(current->deviceId, deviceId) == 0)
		{
			if (current->level < 10)
			{
				digitalWrite(current->commandGpio, !digitalRead(current->commandGpio));
			}
			else
			{
				digitalWrite(current->commandGpio, LOW);
			}

			sendTankLevelNotification(current);
			return 1;
		}

		current = current->next;
	}

	return 0;
}

void timerCallbackTankLevel(struct tm *timeinfo)
{
	TankLevelList *current = firstTankLevel;

	while (current != NULL)
	{
		int value = digitalRead(current->commandGpio);
		if (value == HIGH)
		{
			digitalWrite(current->notifyGpio, state);
		}
		else
		{
			digitalWrite(current->notifyGpio, LOW);
		}

		current = current->next;
	}
}
