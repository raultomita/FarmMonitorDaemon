#include <wiringPi.h>
#include <stdio.h>
#include "tankLevel.h"

const int ledPinTankFull = 18;
const int ledPinTankMedium = 23;
const int ledPinTankEmpty = 24;
const int ledPinTankInputEv = 22;
const int btnPinDrain = 27;
const int btnPinFill=17;
volatile int tankState = 0;

void writeTankLedValues(int emptyState, int mediumState, int fullState){
	digitalWrite(ledPinTankEmpty, emptyState);
	digitalWrite(ledPinTankMedium, mediumState);
	digitalWrite(ledPinTankFull, fullState);
}

void displayTankLevel(void){
	switch(tankState){
		case 0: 
			writeTankLedValues(HIGH, LOW, LOW); 
			break;
		case 1: 
			writeTankLedValues(HIGH, HIGH, LOW); 
			break;
		case 2:
			writeTankLedValues(LOW, HIGH, LOW); 
			break;
		case 3:
			writeTankLedValues(LOW, HIGH, HIGH); 
			break;
		case 4:
			writeTankLedValues(LOW, LOW, HIGH); 
			break;
	}
}

void triggerElectroValve(void){
	if(tankState <= 1)
	{
		digitalWrite(ledPinTankInputEv, HIGH);
	}
	else if(tankState == 4)
	{
		digitalWrite(ledPinTankInputEv, LOW);
	}
}

//Interrupts callbacks
void fillTankLevel(void){
	if(tankState < 4)
	{
		tankState += 1;
		printf("fill tank state is: %d\n", tankState);
		displayTankLevel();
		triggerElectroValve();
	}
}

void drainTankLevel(void){
	if(tankState >0){
		tankState -= 1;
		printf("drain tank state is: %d\n", tankState);
		displayTankLevel();
		triggerElectroValve();
	}
}

//Public APIs
void initializeTankLevel(void)
{
	pinMode(ledPinTankEmpty, OUTPUT);
	pinMode(ledPinTankMedium, OUTPUT);
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
	return tankState*25;
}
