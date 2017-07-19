#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include "tankLevel.h"
#include "watering.h"
#include "display.h"
#include "external.h"
#include "notification.h"
#include "pins.h"

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
	initializeNotification();
	printf("[%ld] ConfigurationComplete\n", pthread_self());
	//void saveAndNotify(char * key, char* data)
	//do some idle work
	time_t rawtime;
	struct tm* timeinfo;
	
	while(1)
	{
		delay(1000);
		
		state = !state;
		
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		
		timerCallbackWatering(timeinfo);
		timerCallbackTankLevel(timeinfo);	
	}
    return 0;
}
