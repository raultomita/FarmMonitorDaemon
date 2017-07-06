#include <wiringPi.h>
#include "display.h"
#include "tankLevel.h"
#include "pins.h"

volatile int tankState = 0;
const int emptyCell = '_';
const int fullCell = '+';
char levelMessage[10] = "__________";
 
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
	displayMessage(1, 7, levelMessage);
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
	displayMessage(1, 0, "Water:");
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
