#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "notification.h"

const char *switchJsonFormat =
	"{ \"id\": \"switch%d\", \"type\": \"switch\", \"timeStamp\": \"%s\", \"state\": \"%d\" }";

struct SwitchItem {
	char *deviceId;
	char *display;
	char *location;
	int gpio;
	SwitchItem *next;
} *firstSwitch, *lastSwitch;

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

void toggleSwitch(char* switchId){
	SwitchItem *current;
	while
	digitalWrite(switchPins[number], !digitalRead(switchPins[number]));
	sendSwitchNotification(number);
}

//Public APIs
void addSwitch(char *switchId, char *display, char *location, int gpio)
{
	SwitchItem *newDevice = malloc(sizeof(SwitchItem));
	newDevice->deviceId = malloc(strlen(swithcId) * sizeof(char));
	strcpy(newDevice->deviceId, switchId);
	newDevice->gpio = gpio;

	if(firstSwitch == NULL){
		*firstSwitch = *newDevice;
		*lastSwitch = *newDevice;	
	}

// int i =0;
//   for(i = 0; i< 8; i++){
//     	pinMode(switchPins[i], OUTPUT);
//     	digitalWrite(switchPins[i], LOW);
// 	sendSwitchNotification(i);
//   }	
}

