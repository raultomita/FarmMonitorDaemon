#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "notification.h"

const char *switchJsonFormat =
	"{ \"id\": \"switch%s\", \"type\": \"switch\", \"timeStamp\": \"%s\", \"state\": \"%d\" }";

void sendSwitchNotification(int number)
{
	char timeString[18];
	getCurrentTimeInfo(timeString, sizeof(timeString));
  
	char *json = (char *)malloc((strlen(switchJsonFormat) + strlen(timeString)+1) * sizeof(char));
	sprintf(json,
			switchJsonFormat,
      number,
			timeString,
			digitalRead(switchPins[number]));
	saveAndNotify("tankLevel", json);
}

//Public APIs
void initializeSwitches(void)
{
  for(int i = 0; i< 8; i++){
    pinMode(switchPins[i], OUTPUT);
    digitalWrite(commandPinTankInputEv, LOW);
  }	
}
}
