#include <wiringPi.h>
#include <time.h>
#include "display.h"
#include "tankLevel.h"
#include "pins.h"

volatile int tankState = 0;
const int emptyCell = '_';
const int fullCell = '+';
char levelMessage[10] = "__________";
 
void displayTankLevel(void){	
	if(tankState < 10)
	{
		digitalWrite(ledPinTankFull, LOW);
	}
	else
	{
		digitalWrite(ledPinTankFull, HIGH);
	}	
	displayMessage(0, 7, levelMessage);
}

void triggerElectroValve(void){
	if(tankState <= 3)
	{
		digitalWrite(commandPinTankInputEv, HIGH);
	}
	else if(tankState == 10)
	{
		digitalWrite(commandPinTankInputEv, LOW);
	}
}

//Interrupts callbacks
void fillTankLevel(void){
	if(tankState < 10)
	{
		
		tankState += 1;
		levelMessage[tankState-1] = fullCell;		
		
		displayTankLevel();
		triggerElectroValve();
	}
}

void drainTankLevel(void){
	if(tankState >0 ){
		levelMessage[tankState-1] = emptyCell;
		tankState -= 1;
		
		displayTankLevel();
		triggerElectroValve();
	}
}

//Public APIs
void initializeTankLevel(void)
{	
	pinMode(ledPinTankFull, OUTPUT);
	pinMode(ledPinTankInputEvOperation, OUTPUT);
	pinMode(commandPinTankInputEv, OUTPUT);
 	pinMode(btnPinFill, INPUT);
 	pinMode(btnPinDrain, INPUT);
	pullUpDnControl(btnPinFill, PUD_UP);
	pullUpDnControl(btnPinDrain, PUD_UP);
	wiringPiISR(btnPinFill, INT_EDGE_RISING, &fillTankLevel);
	wiringPiISR(btnPinDrain, INT_EDGE_RISING, &drainTankLevel);
	//Query tank lavel from sensor
	displayMessage(0, 0, "Water:");
	displayTankLevel();
	triggerElectroValve();
}

int getTankLevel(void)
{
	return tankState*10;
}

void timerCallbackTankLevel(struct tm * timeinfo)
{
	int value = digitalRead(commandPinTankInputEv);
	if(value == HIGH)
	{	
		digitalWrite(ledPinTankInputEvOperation, state);
	}
	else
	{
		digitalWrite(ledPinTankInputEvOperation, LOW);
	}
}
