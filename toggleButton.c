#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "toggleButton.h"
#include "switch.h"
#include "tankLevel.h"
#include "watering.h"

    typedef struct ToggleButton
{
    char *deviceId;
    int gpio;
    char *targetDeviceId;

    struct ToggleButton *next;
} ToggleButtonList;
ToggleButtonList *firstToggleButton, *lastToggleButton;

void toggleTargetDeviceId(char *targetDeviceId)
{
    if (strncmp("tankLevel", targetDeviceId, strlen("tankLevel")) == 0)
    {
        triggerTankLevel(targetDeviceId);
    }
    else if (strncmp("switch", targetDeviceId, strlen("switch")) == 0)
    {
        toggleSwitch(targetDeviceId);
    }
    else if (strncmp("watering", targetDeviceId, strlen("watering")) == 0)
    {
        triggerWatering(targetDeviceId);
    }
}

void addToggleButton(char *toggleButtonId, int gpio, char *targetDeviceId)
{
    ToggleButtonList *newDevice = malloc(sizeof(ToggleButtonList));
    newDevice->deviceId = malloc(strlen(toggleButtonId) * sizeof(char));
    strcpy(newDevice->deviceId, toggleButtonId);

    newDevice->targetDeviceId = malloc(strlen(targetDeviceId) * sizeof(char));
    strcpy(newDevice->targetDeviceId, targetDeviceId);

    newDevice->gpio = gpio;

    if (firstToggleButton == NULL)
    {
        firstToggleButton = newDevice;
        lastToggleButton = newDevice;
    }
    else
    {
        lastToggleButton->next = newDevice;
        lastToggleButton = newDevice;
    }

    pinMode(newDevice->gpio, INPUT);
    pullUpDnControl(newDevice->gpio, PUD_UP);
    //wiringPiISR(btnPinFill, INT_EDGE_RISING, &fillTankLevel);
}
