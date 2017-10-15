#include <wiringPi.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hiredis/hiredis.h>

#include "main.h"
#include "devices/devices.h"

char *redisHost = "192.168.1.200";
int redisPort = 6379;
int state = LOW;

char *instanceId;

void getCurrentTimeInfo(char *timeString, int bufferSize)
{
	time_t rawTime;
	struct tm *timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(timeString, bufferSize, "%x %X", timeInfo);
}

int triggerInternalDevice(char *deviceMessage)
{

	triggerTankLevel(deviceMessage);
	toggleSwitch(deviceMessage);
	setNightWithness(deviceMessage);
	triggerWatering(deviceMessage);
}

void initializeSwitch(char *deviceId, redisReply *r)
{
	logInfo("Init switch with id %s", deviceId);

	if (r->elements == 10 && strcmp(r->element[6]->str, "gpio") == 0)
	{
		addSwitch(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10));
	}
}

void initializeTankLevel(char *deviceId, redisReply *r)
{
	logInfo("Init switch with id %s", deviceId);

	if (r->elements == 12 && strcmp(r->element[6]->str, "commandGpio") == 0 &&
		strcmp(r->element[8]->str, "notifyGpio") == 0 && strcmp(r->element[10]->str, "levelGpio") == 0)
	{
		addTankLevel(deviceId, r->element[3]->str, r->element[5]->str,
					 strtoimax(r->element[7]->str, NULL, 10),
					 strtoimax(r->element[9]->str, NULL, 10),
					 strtoimax(r->element[11]->str, NULL, 10));
	}
}

void initializeWatering(char *deviceId, redisReply *r)
{
	logInfo("Init watering with id %s", deviceId);

	if (r->elements == 10 && strcmp(r->element[6]->str, "commandGpio") == 0 && strcmp(r->element[8]->str, "notifyGpio") == 0)
	{
		addWatering(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10), strtoimax(r->element[9]->str, NULL, 10));
	}
}

void initializeToggleButton(char *deviceId, redisReply *r)
{
	logInfo("Init toggle button with id %s", deviceId);

	if (r->elements == 8 && strcmp(r->element[2]->str, "gpio") == 0 && strcmp(r->element[4]->str, "targetDeviceId") == 0)
	{
		addToggleButton(deviceId, strtoimax(r->element[3]->str, NULL, 10), strtoimax(r->element[7]->str, NULL, 10), r->element[5]->str);
	}
}

void initializeDevice(char *deviceId, redisReply *r)
{
	if (r->type == REDIS_REPLY_ARRAY &&
		r->elements > 1 &&
		strcmp(r->element[0]->str, "type") == 0)
	{
		if (strcmp(r->element[1]->str, "switch") == 0)
		{
			initializeSwitch(deviceId, r);
		}
		else if (strcmp(r->element[1]->str, "tankLevel") == 0)
		{
			initializeTankLevel(deviceId, r);
		}
		else if (strcmp(r->element[1]->str, "watering") == 0)
		{
			initializeWatering(deviceId, r);
		}
		else if (strcmp(r->element[1]->str, "toggleButton") == 0)
		{
			initializeToggleButton(deviceId, r);
		}
	}
}

int main(void)
{
	//Wait for redis to start. Later will be with containers and we no longer need this line of code.
	delay(2000);
	//Initializes pins as GPIO numbers
	wiringPiSetupGpio();

	initSwitch();
	initToggleButton();
	initWatering();
	initTankLevel();

	initializeRedis();

	logInfo("[Main] Init completed");

	//do some idle work
	time_t rawtime;
	struct tm *timeInfo;

	while (1)
	{
		state = !state;

		time(&rawtime);
		timeInfo = localtime(&rawtime);

		timerCallbackWatering(timeInfo);
		timerCallbackTankLevel(timeInfo);

		delay(1000);
	}
	return 0;
}
