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

pthread_cond_t notificationCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t notificationMutex = PTHREAD_MUTEX_INITIALIZER;

int state = LOW;

char *getCurrentTimeInfo(void)
{
	time_t rawTime;
	struct *tm timeInfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	char *timeString = (char *)malloc(18 * sizeof(char));
	strftime(timeString, sizeof(timeString), "%x %X", timeinfo);
	return timeString;
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

	initializeExternalHandlers();
	delay(100);
	initializeNotification();
	printf("[%ld] ConfigurationComplete\n", pthread_self());

	//do some idle work
	time_t rawtime;
	struct *tm timeInfo;
	
	while (1)
	{
		state = !state;

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		timerCallbackWatering(timeinfo);
		timerCallbackTankLevel(timeinfo);

		delay(1000);
	}
	return 0;
}
