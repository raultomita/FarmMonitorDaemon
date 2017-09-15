#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tankLevel.h"
#include "main.h"
#include "notification.h"

int tankLevelInitialized = 0;
volatile int tankState = 0;
const int emptyCell = '_';
const int fullCell = '+';
char levelMessage[10] = "__________";
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

	struct TankLevel *next;
} TankLevel;
TankLevel *firstTankLevel, *lastTankLevel;

void sendTankLevelNotification(void)
{
	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));

	printf("%s\n", timeString);

	char *json = (char *)malloc((strlen(tankLevelJsonFormat) + strlen(timeString)) * sizeof(char));
	sprintf(json,
			tankLevelJsonFormat,
			timeString,
			tankState,
			digitalRead(commandPinTankInputEv));
	saveAndNotify("tankLevel", json);
}

void displayTankLevel(void)
{
	if (tankState < 10)
	{
		digitalWrite(ledPinTankFull, LOW);
	}
	else
	{
		digitalWrite(ledPinTankFull, HIGH);
	}
	printf(levelMessage);
}

void triggerElectroValve(void)
{
	if (tankState <= 3)
	{
		digitalWrite(commandPinTankInputEv, HIGH);
	}
	else if (tankState == 10)
	{
		digitalWrite(commandPinTankInputEv, LOW);
	}
}

//Interrupts callbacks
void fillTankLevel(void)
{
	if (tankState < 10)
	{

		tankState += 1;
		levelMessage[tankState - 1] = fullCell;

		displayTankLevel();
		triggerElectroValve();
		sendTankLevelNotification();
	}
}

void drainTankLevel(void)
{
	if (tankState > 0)
	{
		levelMessage[tankState - 1] = emptyCell;
		tankState -= 1;

		displayTankLevel();
		triggerElectroValve();
		sendTankLevelNotification();
	}
}

//Public APIs
void addTankLevel(char *tankLevelId, char *display, char *location, int commandGpio, int notifyGpio, int levelGpio)
{
	pinMode(ledPinTankFull, OUTPUT);
	pinMode(ledPinTankInputEvOperation, OUTPUT);
	pinMode(commandPinTankInputEv, OUTPUT);
	pinMode(btnPinFill, INPUT);
	pinMode(btnPinDrain, INPUT);
	pullUpDnControl(btnPinFill, PUD_UP);
	pullUpDnControl(btnPinDrain, PUD_UP);
	wiringPiISR(btnPinFill, INT_EDGE_RISING, &fillTankLevel);
	wiringPiISR(btnPinDrain, INT_EDGE_RISING, &drainTankLevel);
	//Query tank lavel from sensor
	displayTankLevel();
	triggerElectroValve();
}

int getTankLevel(void)
{
	return tankState * 10;
}

void triggerTankLevel(void)
{
	if (tankState < 10)
	{
		digitalWrite(commandPinTankInputEv, !digitalRead(commandPinTankInputEv));
	}
	else
	{
		digitalWrite(commandPinTankInputEv, LOW);
	}

	sendTankLevelNotification();
}

void timerCallbackTankLevel(struct tm *timeinfo)
{
	if (!tankLevelInitialized)
	{
		sendTankLevelNotification();
		tankLevelInitialized = 1;
	}

	int value = digitalRead(commandPinTankInputEv);
	if (value == HIGH)
	{
		digitalWrite(ledPinTankInputEvOperation, state);
	}
	else
	{
		digitalWrite(ledPinTankInputEvOperation, LOW);
	}
}
