#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>

#include "../main.h"
#include "devices.h"

typedef struct AutomaticTrigger
{
	char *deviceId;	
	char *targetDeviceId;
    char *listenOnDeviceId;
	struct AutomaticTrigger *next;
	long switchOnAt;

} AutomaticTriggerList;
AutomaticTriggerList *firstAutomaticTrigger, *lastAutomaticTrigger;


void addautomaticTrigger(char *automaticTriggerId, char *targetDeviceId, char * listenOnDeviceId)
{
	AutomaticTriggerList *newDevice = malloc(sizeof(AutomaticTriggerList));
	newDevice->deviceId = malloc(strlen(automaticTriggerId) * sizeof(char));
	strcpy(newDevice->deviceId, automaticTriggerId);

	newDevice->targetDeviceId = malloc(strlen(targetDeviceId) * sizeof(char));
	strcpy(newDevice->targetDeviceId, targetDeviceId);

    newDevice->listenOnDeviceId = malloc(strlen(listenOnDeviceId) * sizeof(char));

	if (firstAutomaticTrigger == NULL)
	{
		firstAutomaticTrigger = newDevice;
		lastAutomaticTrigger = newDevice;
	}
	else
	{
		lastAutomaticTrigger->next = newDevice;
		lastAutomaticTrigger = newDevice;
	}

	newDevice->switchOnAt = 0;
	newDevice->next = NULL;
}

void scheduleAutomaticTrigger(char *listenOnDeviceId){
	int indexOfColon = strcspn(listenOnDeviceId, ":");
	logDebug("[AutomaticTrigger] index of colon %d", indexOfColon);
	AutomaticTriggerList *current = firstAutomaticTrigger;
	while (current != NULL)
	{
		if (strncmp(current->listenOnDeviceId, listenOnDeviceId, indexOfColon) == 0)
		{
			logDebug("[AutomaticTrigger] led should be notified after this step %s and state ", current->listenOnDeviceId);
			if (listenOnDeviceId[indexOfColon + 1] == '0')
			{
				if(current.switchOnAt == -1){
					triggerDevice(current->targetDeviceId);			
				
				logDebug("[AutomaticTrigger] Trigger device: %ld", current->targetDeviceId);
				current->switchOnAt = 0;				
			}
			else
			{
				time_t rawtime;
				time(&rawtime);			

				logDebug("[AutomaticTrigger] Start counting: %ld", rawtime);

			current->switchOnAt = (long)rawtime;
			}
			return 1;
		}

		current = current->next;
	}

	return 0;
}

void timerCallbackAutomaticTrigger(time_t rawtime)
{
	AutomaticTriggerList *current = firstAutomaticTrigger;
	while (current != NULL)
	{
		if(current.switchOnAt > 0 && ((long)rawtime - current->switchOnAt) > 5){
			triggerDevice(current->targetDeviceId);
			logDebug("[AutomaticTrigger] Trigger device: %ld", current->targetDeviceId);
			current.switchOnAt = -1;
		}

		current = current->next;
	}	
}