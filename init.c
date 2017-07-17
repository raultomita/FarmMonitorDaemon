#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include "tankLevel.h"
#include "watering.h"
#include "display.h"
#include "external.h"
#include "pins.h"

int ledPinTankFull = 23;

int commandPinTankInputEv = 17;
int commandPinTankOutputEv = 24;

int ledPinTankInputEvOperation = 18;
int ledPinTankOutputEvOperation = 25;

int btnPinDrain = 22;
int btnPinFill = 27;

int state = LOW;

int main(void)
{
	delay(1000);
	
	//Initialize redis and subscribe to external event on separate thread
	new thread
	{
		initializeRedis();
	}
	//wait ofor it
	
	//Initializes pins and main modules
	//Write their initial state
	wiringPiSetupGpio();
	initializeDisplay();		
	initializeTankLevel();	
	initializeWateringSchedule();
		
	printf("ConfigurationComplete\n");
	
	//do some idle work
	time_t rawtime;
	struct tm* timeinfo;
	
	while(1)
	{
		delay(1000);
		
		state = !state;
		
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
				
		displayMessage(3, 0, asctime(timeinfo)); 
		
		timerCallbackWatering(timeinfo);
		timerCallbackTankLevel(timeinfo);	
	}
    return 0;
}
