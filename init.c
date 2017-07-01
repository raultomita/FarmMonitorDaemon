#include <wiringPi.h>
#include <stdio.h>

const int ledPinPwm = 18;
const int ledPin = 17;
const int btnPin = 23;

int main(void)
{
	wiringPiSetupGpio();

	pinMode(ledPin, OUTPUT);
	pinMode(btnPin, INPUT);
	pullUpDnControl(btnPin, PUD_UP);
	printf("Configuration complete \n");

	digitalWrite(ledPin, HIGH);
	printf("Write values\n");

	while(1)
	{
		printf("Heartbeat\n");
		delay(1000);

		int readValue = digitalRead(btnPin);
		printf("Value is:%d\n",readValue);
		if(readValue){
		   digitalWrite(ledPin, LOW);
		}
		else
		{
		   digitalWrite(ledPin, HIGH);
		}
	}

        return 0;
}
