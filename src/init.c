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
int debug = 0;

char *instanceId;

void getCurrentTimeInfo(char *timeString, int bufferSize)
{
	time_t rawTime;
	struct tm *timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(timeString, bufferSize, "%x %X", timeInfo);
}

int main(void)
{
	//Wait for redis to start. Later will be with containers and we no longer need this line of code.
	delay(2000);
	//Initializes pins as GPIO numbers
	wiringPiSetupGpio();

	initializeDispatcher();

	initializeRedis();

	logInfo("[Main] All initialization methods called. do some idle work");

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
