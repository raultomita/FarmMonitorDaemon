#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include "tankLevel.h"
#include "watering.h"
#include "display.h"
#include "external.h"
#include "notification.h"
#include "main.h"

int ledPinTankFull = 23;

int commandPinTankInputEv = 17;
int commandPinTankOutputEv = 24;

int ledPinTankInputEvOperation = 18;
int ledPinTankOutputEvOperation = 25;

int btnPinDrain = 22;
int btnPinFill = 27;

char switchPins[8] = [26, 16, 13, 12, 6, 5, 1, 0];

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

int main(void)
{
	delay(1000);

	//Initializes pins and main modules
	//Don't write their initial state
	wiringPiSetupGpio();
	initializeDisplay();
	initializeTankLevel();
	initializeWateringSchedule();
	initializeSwitches();
	
	initializeExternalHandlers();
	delay(100);
	initializeNotification();
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
