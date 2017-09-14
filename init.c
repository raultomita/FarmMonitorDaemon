#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hiredis/hiredis.h>

#include "tankLevel.h"
#include "switch.h"
#include "watering.h"

#include "external.h"
#include "notification.h"

#include "main.h"

char *instanceId = "mainframe";
char *redisHostname = "127.0.0.1";

int ledPinTankFull = 23;

int commandPinTankInputEv = 17;
int commandPinTankOutputEv = 24;

int ledPinTankInputEvOperation = 18;
int ledPinTankOutputEvOperation = 25;

int btnPinDrain = 22;
int btnPinFill = 27;

char switchPins[8] = {26, 16, 13, 12, 6, 5, 1, 0};

pthread_cond_t notificationCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t notificationMutex = PTHREAD_MUTEX_INITIALIZER;

int state = LOW;

void getCurrentTimeInfo(char *timeString, int bufferSize)
{
	time_t rawTime;
	struct tm *timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(timeString, bufferSize, "%x %X", timeInfo);
	printf("%s\n", timeString);
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

void initializeSwitch(char * deviceId, redisReply *r)
{
	printf("Init switch with id %s\n", deviceId);
	
	if (r->elements == 8 && strcmp(r->element[6]->str, "gpio") == 0)
	{
		addSwitch(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10));
	}
}

void initializeTankLevel(char * deviceId, redisReply *r)
{
	printf("Init switch with id %s\n", deviceId);
	
	if (r->elements == 8 && strcmp(r->element[6]->str, "gpio") == 0)
	{
		//addTankLevel(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10));
	}
}

void initializeWatering(char * deviceId, redisReply *r)
{
	printf("Init watering with id %s\n", deviceId);
	
	if (r->elements == 10 && strcmp(r->element[6]->str, "commandGpio") == 0 && strcmp(r->element[8]->str, "notifyGpio") == 0)
	{
		addWatering(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10), strtoimax(r->element[9]->str, NULL, 10));
	}
}

void initializeSwitchButton(char * deviceId, redisReply *r)
{
	printf("Switch button not supported with id %s\n", deviceId);
	
	
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
				else if(strcmp(replyDeviceId->element[1]->str, "tankLevel") == 0){
					initializeTankLevel(r->element[1]->element[dataIndex]->str, replyDeviceId);
				}
				else if(strcmp(replyDeviceId->element[1]->str, "watering") == 0){
					initializeWatering(r->element[1]->element[dataIndex]->str, replyDeviceId);
				}
				else if(strcmp(replyDeviceId->element[1]->str, "switchButton") == 0){
					//InitializeSwitchButton(r->element[1]->element[dataIndex]->str, replyDeviceId);
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
	//Wait for redis to start. Later will be with containers
	delay(2000);
	//Initializes pins as GPIO numbers
	wiringPiSetupGpio();
	initializeNotification();

	redisContext *c = redisConnect(redisHostname, 6379);
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

	initializeExternalHandlers();
	delay(100);
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
		//timerCallbackTankLevel(timeInfo);

		delay(1000);
	}
	return 0;
}
