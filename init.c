#include <wiringPi.h>
#include <stdio.h>
#include "tankLevel.h"

const int ledPinPwm = 18;
const int ledPin2 = 27;
const int ledPin = 17;

const int btnPin = 23;
const int btnPinRising = 24;
const int btnPin3 = 22;

void buttonPushed(void){
	digitalWrite(ledPin, HIGH);
	digitalWrite(ledPin2, LOW);
}

void buttonPushedRising(void){
	digitalWrite(ledPin, LOW);
	digitalWrite(ledPin2, HIGH);
}

int main(void)
{
	wiringPiSetupGpio();

	pinMode(ledPin, OUTPUT);
	pinMode(ledPin2, OUTPUT);

	pinMode(btnPinRising, INPUT);
	pinMode(btnPin, INPUT);

	pullUpDnControl(btnPinRising, PUD_UP);
	pullUpDnControl(btnPin, PUD_UP);

	wiringPiISR(btnPin, INT_EDGE_RISING, &buttonPushed);
	wiringPiISR(btnPinRising, INT_EDGE_RISING, &buttonPushedRising);
	printf("Configuration complete \n");

	while(1){
		printf("I'm alive\n");
		delay(60000);
	}
        return 0;
}
