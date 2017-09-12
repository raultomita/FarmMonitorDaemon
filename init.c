#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include "tankLevel.h"
#include "switch.h"
#include "watering.h"
#include "display.h"
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

void initializeDevices(redisContext *c, char* cursor)
{
	printf("Start device initialization\n");

	redisReply *r;
	reply = redisCommand(c, "SSCAN %s %s", instanceId, cursor);

	if (r->type != REDIS_REPLY_ARRAY)
	{
		printf("Response for SSCAN is not an array\n");
		freeReplyObject(r);
		return;
	}

	if (r->elements == 2)
	{
		printf("Array does not contains two elements\n");
		freeReplyObject(r);
		return;
	}

	if (r->element[1] != NULL && r->element[1]->type == REDIS_REPLY_ARRAY)
	{
		redisReply *replyDeviceId;
		int dataIndex;
		for (dataIndex = 0; dataIndex < r->element[1]->elements; dataIndex++)
		{
			replyDeviceId = redisCommand(c, "HGETALL %s", r->element[1]->element[dataIndex]->str);
			if(replyDeviceId->type == REDIS_REPLY_ARRAY && replyDeviceId->elements > 1 && replyDeviceId->element[0]->str == "type")
			{
				if(replyDeviceId->element[1]->str == "switch"){
					printf("Init switch %s\n", r->element[1]->element[dataIndex]->str);
				}
			}
			freeReplyObject(replyDeviceId);
		}
	}

	if (->element[0] != NULL && r->element[0]->str != "0")
	{
		initializeDevices(c, r->element[0]->str);
	}

	freeReplyObject(r);
}

int main(void)
{
	//Wait for redis to start. Later will be with containers
	delay(2000);
	//Initializes pins as GPIO numbers
	wiringPiSetupGpio();

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

	//initializeTankLevel();
	//initializeWateringSchedule();
	//initializeSwitches();

	//initializeExternalHandlers();
	//delay(100);
	//initializeNotification();
	printf("[%ld] ConfigurationComplete\n", pthread_self());

	//do some idle work
	time_t rawtime;
	struct tm *timeInfo;

	while (1)
	{
		state = !state;

		time(&rawtime);
		timeInfo = localtime(&rawtime);

		//timerCallbackWatering(timeInfo);
		//timerCallbackTankLevel(timeInfo);

		delay(1000);
	}
	return 0;
}
