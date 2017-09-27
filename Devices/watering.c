#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "watering.h"
#include "tankLevel.h"
#include "notification.h"
#include "main.h"

const char *wateringStateJsonFormat =
	"{ \"id\": \"%s\", \"type\": \"watering\", \"display\":\"%s\", \"location\":\"%s\", \"timeStamp\":\"%s\", \"state\": \"%d\" }";

typedef struct Watering
{
	char *deviceId;
	char *display;
	char *location;
	int commandGpio;
	int notifyGpio;
	struct tm *wateringSystemOverridedOn;

	struct Watering *next;
} WateringList;
WateringList *firstWatering, *lastWatering;

void sendWateringNotification(WateringList *watering)
{
	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));

	printf("%s\n", timeString);

	char *json = (char *)malloc((strlen(wateringStateJsonFormat) + strlen(timeString) + strlen(watering->deviceId) +
								 strlen(watering->display) + strlen(watering->location)) *
								sizeof(char));
	sprintf(json, wateringStateJsonFormat,
			watering->deviceId,
			watering->display,
			watering->location,
			timeString,
			digitalRead(watering->commandGpio));
	saveAndNotify(watering->deviceId, json);
}

void addWatering(char *wateringId, char *display, char *location, int commandGpio, int notifyGpio)
{
	WateringList *newDevice = malloc(sizeof(WateringList));
	newDevice->deviceId = malloc(strlen(wateringId) * sizeof(char));
	strcpy(newDevice->deviceId, wateringId);

	newDevice->display = malloc(strlen(display) * sizeof(char));
	strcpy(newDevice->display, display);

	newDevice->location = malloc(strlen(location) * sizeof(char));
	strcpy(newDevice->location, location);

	newDevice->commandGpio = commandGpio;
	newDevice->notifyGpio = notifyGpio;

	if (firstWatering == NULL)
	{
		firstWatering = newDevice;
		lastWatering = newDevice;
	}
	else
	{
		lastWatering->next = newDevice;
		lastWatering = newDevice;
	}

	pinMode(newDevice->commandGpio, OUTPUT);
	pinMode(newDevice->notifyGpio, OUTPUT);
	digitalWrite(newDevice->commandGpio, LOW);
	digitalWrite(newDevice->notifyGpio, LOW);
        sendWateringNotification(newDevice);
}

int hasSoilEnoughWater(void)
{
	//TODO: when I have a moisture sensor this method should return the actual state of the soil
	return 0;
}

void timerCallbackWatering(struct tm *timeinfo)
{
	WateringList *current = firstWatering;

	while (current != NULL)
	{
		int wateringState = digitalRead(current->commandGpio);
		if (current->wateringSystemOverridedOn != NULL)
		{
			//TODO: take over control  after 12 h
		}
		else if (timeinfo->tm_sec >= 15 && timeinfo->tm_sec <= 45 && !hasSoilEnoughWater())
		{
			wateringState = HIGH;
		}
		else
		{
			wateringState = LOW;
		}

		if (wateringState != digitalRead(current->commandGpio))
		{
			digitalWrite(current->commandGpio, wateringState);
			sendWateringNotification(current);
		}

		if (digitalRead(current->commandGpio))
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

void triggerWatering(char *deviceId)
{
	WateringList *current = firstWatering;

	while (current != NULL)
	{
		if (strcmp(current->deviceId, deviceId) == 0)
		{
			time_t rawtime;
			time(&rawtime);
			current->wateringSystemOverridedOn = localtime(&rawtime);

			int currentState = digitalRead(current->commandGpio);
			digitalWrite(current->commandGpio, !currentState);
			digitalWrite(current->notifyGpio, !currentState);
			sendWateringNotification(current);

			return;
		}

		current = current->next;
	}
}
