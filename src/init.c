#include <wiringPi.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <hiredis/hiredis.h>

#include "main.h"
#include "devices/devices.h"

char *redisHost = "192.168.1.200";
int redisPort = 6379;
int state = LOW;
int debug = 0;
int arg;

char *instanceId;

void getCurrentTimeInfo(char *timeString, int bufferSize)
{
	time_t rawTime;
	struct tm *timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(timeString, bufferSize, "%x %X", timeInfo);
}

int main(int argc, char **argv)
{
	while ((arg = getopt(argc, argv, "d")) != -1)
	{
		switch (arg)
		{
		case 'd':
			debug = 1;
			printf("Debug is enabled\n");
		}
	}

	//Initializes pins as GPIO numbers
	wiringPiSetupGpio();

	initializeDispatcher();

	initializeRedis();

	logInfo("[Main] All initialization methods called. do some idle work");

	//do some idle work
	time_t rawtime;
	struct tm *timeInfo;
	time(&rawtime);
	long lastHeartBeat = (long)rawtime;

	while (1)
	{
		state = !state;

		time(&rawtime);
		timeInfo = localtime(&rawtime);

		timerCallbackWatering(timeInfo);
		timerCallbackTankLevel(timeInfo);

		if (((long)rawtime - lastHeartBeat) > 10)
		{
			lastHeartBeat = (long)rawtime;
			sendCommand("heartbeat");
		}
		delay(1000);
	}
	return 0;
}
