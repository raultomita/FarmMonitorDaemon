#include <wiringPi.h>
#include <stdio.h>
#include "tankLevel.h"
#include "pins.h"

volatile int tankState = 0;

void writeTankLedValues(int emptyState, int fullState){
	digitalWrite(ledPinTankEmpty, emptyState);
	digitalWrite(ledPinTankFull, fullState);
}

void displayTankLevel(void){	
	if(tankState == 0)
	{
		writeTankLedValues(HIGH, LOW); 
	}
	else if(tankState < 10)
	{
		writeTankLedValues(HIGH, HIGH);
	}
	else
	{
		writeTankLedValues(LOW, HIGH);
	}	
}

void triggerElectroValve(void){
	if(tankState <= 3)
	{
		digitalWrite(ledPinTankInputEv, HIGH);
	}
	else if(tankState == 10)
	{
		digitalWrite(ledPinTankInputEv, LOW);
	}
}

//Interrupts callbacks
void fillTankLevel(void){
	if(tankState < 10)
	{
		tankState += 1;
		printf("Filling tank: %d\n", tankState);
		displayTankLevel();
		triggerElectroValve();
	}
}

void drainTankLevel(void){
	if(tankState >0 ){
		tankState -= 1;
		printf("draining tank: %d\n", tankState);
		displayTankLevel();
		triggerElectroValve();
	}
}

//Public APIs
void initializeTankLevel(void)
{
	pinMode(ledPinTankEmpty, OUTPUT);	
	pinMode(ledPinTankFull, OUTPUT);
	pinMode(ledPinTankInputEv, OUTPUT);
 	pinMode(btnPinFill, INPUT);
 	pinMode(btnPinDrain, INPUT);
	pullUpDnControl(btnPinFill, PUD_UP);
	pullUpDnControl(btnPinDrain, PUD_UP);
	wiringPiISR(btnPinFill, INT_EDGE_RISING, &fillTankLevel);
	wiringPiISR(btnPinDrain, INT_EDGE_RISING, &drainTankLevel);
	//Query tank lavel from sensor
	displayTankLevel();
	triggerElectroValve();
}

int getTankLevel(void)
{
	return tankState*10;
}

void timerCallbackTankLevel(void)
{
}
