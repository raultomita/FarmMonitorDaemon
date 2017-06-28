#include <wiringPi.h>
#include <stdio.h>

const int ledPin = 23;

int main(void)
{
	wiringPiSetupGpio();
	pinMode(ledPin, OUTPUT);
	
	digitalWrite(ledPin, LOW);
        printf("Some test");
	return 0;
} 
