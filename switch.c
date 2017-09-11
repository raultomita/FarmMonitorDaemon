#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "notification.h"

const char *switchJsonFormat =
	"{ \"id\": \"switch%d\", \"type\": \"switch\", \"timeStamp\": \"%s\", \"state\": \"%d\" }";

void sendSwitchNotification(int number)
{
printf("Switch %d sending\n", number);
	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));
  
	char *json = (char *)malloc((strlen(switchJsonFormat) + strlen(timeString)+1) * sizeof(char));
	sprintf(json,
		switchJsonFormat,
      		number,
		timeString,
		digitalRead(switchPins[number]));
printf("Switch %d notification sent\n", number);
	saveAndNotify("switch", json);
printf("Switch %d notification sent to redis\n", number);
}

void toggleSwitch(int number){
	digitalWrite(switchPins[number], !digitalRead(switchPins[number]));
	sendSwitchNotification(number);
}

//Public APIs
void initializeSwitches(void)
{
int i =0;
  for(i = 0; i< 8; i++){
    	pinMode(switchPins[i], OUTPUT);
    	digitalWrite(switchPins[i], LOW);
	sendSwitchNotification(i);
  }	
}

