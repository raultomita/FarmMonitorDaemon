#include <wiringPi.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hiredis/hiredis.h>

#include "main.h"
#include "devices/devices.h"

char *redisHostname = "127.0.0.1";
int redisPort = 6379;
int state = LOW;

char *instanceId = "mainframe";

void getCurrentTimeInfo(char *timeString, int bufferSize)
{
	time_t rawTime;
	struct tm *timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(timeString, bufferSize, "%x %X", timeInfo);
	printf("%s\n", timeString);
}

void triggerDevice(char *deviceMessage)
{
	int result = triggerInternalDevice(deviceMessage);
	if(result == 0){
		sendMessage(COMMAND, deviceMessge, NULL);
	}
}

int triggerInternalDevice(char *deviceMessage)
{
	if (strncmp("tankLevel", deviceMessage, strlen("tankLevel")) == 0)
	{
		return triggerTankLevel(deviceMessage);
	}
	else if (strncmp("switch", deviceMessage, strlen("switch")) == 0)
	{
		return toggleSwitch(deviceMessage);
	}
	else if (strncmp("watering", deviceMessage, strlen("watering")) == 0)
	{
		return triggerWatering(deviceMessage);
	}

	return 2;//not supported
}

int isInstanceQueryResultValid(redisReply *r)
{
	if (r->type != REDIS_REPLY_ARRAY)
	{
		printf("Response for SSCAN is not an array\n");
		return 0;
	}

	if (r->elements != 2)
	{
		int i = 0;
		for (i = 0; i < r->elements; i++)
		{
			printf("element type %d\n", r->element[i]->type);
		}
		printf("Array does not contains two elements\n");
		return 0;
	}

	if (r->element[1] == NULL || r->element[1]->type != REDIS_REPLY_ARRAY)
	{
		printf("Second element from SSCAN is not an array with device ids\n");
		return 0;
	}

	return 1;
}

int stilExistItems(redisReply *r)
{
	return r->element[0] != NULL && strcmp(r->element[0]->str, "0") != 0;
}

void initializeSwitch(char *deviceId, redisReply *r)
{
	printf("Init switch with id %s\n", deviceId);

	if (r->elements == 8 && strcmp(r->element[6]->str, "gpio") == 0)
	{
		addSwitch(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10));
	}
}

void initializeTankLevel(char *deviceId, redisReply *r)
{
	printf("Init switch with id %s\n", deviceId);

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
	printf("Init watering with id %s\n", deviceId);

	if (r->elements == 10 && strcmp(r->element[6]->str, "commandGpio") == 0 && strcmp(r->element[8]->str, "notifyGpio") == 0)
	{
		addWatering(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10), strtoimax(r->element[9]->str, NULL, 10));
	}
}

void initializeToggleButton(char *deviceId, redisReply *r)
{
	printf("Init toggle button with id %s\n", deviceId);

	if (r->elements == 6 && strcmp(r->element[2]->str, "gpio") == 0 && strcmp(r->element[4]->str, "targetDeviceId") == 0)
	{
		addToggleButton(deviceId, strtoimax(r->element[3]->str, NULL, 10), r->element[5]->str);
	}
}

void initializeDevices(redisContext *c, char *cursor)
{
	printf("Query for device ids for cursor %s\n", cursor);

	redisReply *r = redisCommand(c, "SSCAN %s %s", instanceId, cursor);

	if (isInstanceQueryResultValid(r))
	{
		redisReply *replyDeviceId;
		int dataIndex;
		for (dataIndex = 0; dataIndex < r->element[1]->elements; dataIndex++)
		{
			replyDeviceId = redisCommand(c, "HGETALL %s", r->element[1]->element[dataIndex]->str);
			if (replyDeviceId->type == REDIS_REPLY_ARRAY &&
				replyDeviceId->elements > 1 &&
				strcmp(replyDeviceId->element[0]->str, "type") == 0)
			{
				if (strcmp(replyDeviceId->element[1]->str, "switch") == 0)
				{
					initializeSwitch(r->element[1]->element[dataIndex]->str, replyDeviceId);
				}
				else if (strcmp(replyDeviceId->element[1]->str, "tankLevel") == 0)
				{
					initializeTankLevel(r->element[1]->element[dataIndex]->str, replyDeviceId);
				}
				else if (strcmp(replyDeviceId->element[1]->str, "watering") == 0)
				{
					initializeWatering(r->element[1]->element[dataIndex]->str, replyDeviceId);
				}
				else if (strcmp(replyDeviceId->element[1]->str, "toggleButton") == 0)
				{
					initializeToggleButton(r->element[1]->element[dataIndex]->str, replyDeviceId);
				}
			}
			freeReplyObject(replyDeviceId);
		}

		if (stilExistItems(r))
		{
			initializeDevices(c, r->element[0]->str);
		}
	}

	freeReplyObject(r);
}

int main(void)
{
	//Wait for redis to start. Later will be with containers and we no longer need this line of code.
	delay(2000);
	//Initializes pins as GPIO numbers
	wiringPiSetupGpio();
	initializeRedisPortal();

	redisContext *c = redisConnect(redisHostname, redisPort);
	if (c == NULL || c->err)
	{
		if (c)
		{
			printf("Error in initializing devices: %s\n", c->errstr);
		}
		else
		{
			printf("Can't allocate redis context in initializing devices\n");
		}
	}
	else
	{
		initializeDevices(c, "0");
	}
	redisFree(c);

	acceptIncommingMessages();

	printf("[%ld] ConfigurationComplete\n", pthread_self());

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
