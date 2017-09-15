#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tankLevel.h"
#include "main.h"
#include "notification.h"

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

	saveAndNotify(tankLevel->deviceId, json);
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

	printf("Level is %s\n", tankLevel->level);
}

void triggerElectroValve(TankLevelList *tankLevel)
{
	if (tankLevel->level <= 3)
	{
		digitalWrite(tankLevel->commandGpio, HIGH);
	}
	else if (TankLevel->level == 10)
	{
		digitalWrite(tankLevel->commandGpio, LOW);
	}
}

//Interrupts callbacks
// void fillTankLevel(void)
// {
// 	if (tankState < 10)
// 	{
// 		tankState += 1;
// 		levelMessage[tankState - 1] = fullCell;

// 		displayTankLevel();
// 		triggerElectroValve();
// 		sendTankLevelNotification();
// 	}
// }

// void drainTankLevel(void)
// {
// 	if (tankState > 0)
// 	{
// 		levelMessage[tankState - 1] = emptyCell;
// 		tankState -= 1;

// 		displayTankLevel();
// 		triggerElectroValve();
// 		sendTankLevelNotification();
// 	}
// }

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
	// pinMode(btnPinFill, INPUT);
	// pinMode(btnPinDrain, INPUT);
	// pullUpDnControl(btnPinFill, PUD_UP);
	// pullUpDnControl(btnPinDrain, PUD_UP);
	// wiringPiISR(btnPinFill, INT_EDGE_RISING, &fillTankLevel);
	// wiringPiISR(btnPinDrain, INT_EDGE_RISING, &drainTankLevel);
	//Query tank lavel from sensor
	displayTankLevel(newDevice);
	triggerElectroValve(newDevice);
	digitalWrite(newDevice->notifyGpio, LOW);

}

void triggerTankLevel(char *deviceId)
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
			return;
		}

		current = current->next;
	}
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
