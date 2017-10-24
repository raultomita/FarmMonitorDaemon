#include <wiringPi.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <regex.h>

#include "main.h"
#include "devices/devices.h"

regex_t switchRegex;
regex_t tankLevelRegex;
regex_t toggleButtonRegex;
regex_t wateringRegex;

void initializeDispatcher()
{
    if (regcomp(&switchRegex, "^switch[0-9]*$", REG_EXTENDED))
    {
        logError("[Dispatcher] Regex pattern for switch could not be compiled");
    }

    if (regcomp(&tankLevelRegex, "^tankLevel[0-9]*$", REG_EXTENDED))
    {
        logError("[Dispatcher] Regex pattern for tankLevel could not be compiled");
    }

    if (regcomp(&toggleButtonRegex, "^switch[0-9]*:[01]$", REG_EXTENDED))
    {
        logError("[Dispatcher] Regex pattern for toggleButton could not be compiled");
    }

    if (regcomp(&wateringRegex, "^watering[0-9]*$", REG_EXTENDED))
    {
        logError("[Dispatcher] Regex pattern for watering could not be compiled");
    }
    logDebug("[Dispatcher] Initialiazed");
}

void initializeSwitch(char *deviceId, redisReply *r)
{
    logInfo("[Dispatcher] Init switch with id %s", deviceId);

    if (r->elements == 8 && strcmp(r->element[6]->str, "gpio") == 0)
    {
        addSwitch(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10));
        logInfo("[Dispatcher] Switch with id %s initialized", deviceId);
    }
}

void initializeTankLevel(char *deviceId, redisReply *r)
{
    logInfo("Init tankLevel with id %s", deviceId);

    if (r->elements == 12 && strcmp(r->element[6]->str, "commandGpio") == 0 &&
        strcmp(r->element[8]->str, "notifyGpio") == 0 && strcmp(r->element[10]->str, "levelGpio") == 0)
    {
        addTankLevel(deviceId, r->element[3]->str, r->element[5]->str,
                     strtoimax(r->element[7]->str, NULL, 10),
                     strtoimax(r->element[9]->str, NULL, 10),
                     strtoimax(r->element[11]->str, NULL, 10));
    }
}

void initializeWatering(char *deviceId, redisReply *r)
{
    logInfo("Init watering with id %s", deviceId);

    if (r->elements == 10 && strcmp(r->element[6]->str, "commandGpio") == 0 && strcmp(r->element[8]->str, "notifyGpio") == 0)
    {
        addWatering(deviceId, r->element[3]->str, r->element[5]->str, strtoimax(r->element[7]->str, NULL, 10), strtoimax(r->element[9]->str, NULL, 10));
    }
}

void initializeToggleButton(char *deviceId, redisReply *r)
{
    logInfo("Init toggle button with id %s", deviceId);

    if (r->elements == 8 && strcmp(r->element[2]->str, "gpio") == 0 && strcmp(r->element[4]->str, "targetDeviceId") == 0)
    {
        addToggleButton(deviceId, strtoimax(r->element[3]->str, NULL, 10), strtoimax(r->element[7]->str, NULL, 10), r->element[5]->str);
    }
}

void addDevice(char *deviceId, redisReply *r)
{
    if (r->type == REDIS_REPLY_ARRAY &&
        r->elements > 1 &&
        strcmp(r->element[0]->str, "type") == 0)
    {
        if (strcmp(r->element[1]->str, "switch") == 0)
        {
            initializeSwitch(deviceId, r);
        }
        else if (strcmp(r->element[1]->str, "tankLevel") == 0)
        {
            initializeTankLevel(deviceId, r);
        }
        else if (strcmp(r->element[1]->str, "watering") == 0)
        {
            initializeWatering(deviceId, r);
        }
        else if (strcmp(r->element[1]->str, "toggleButton") == 0)
        {
            initializeToggleButton(deviceId, r);
        }
    }
}

int triggerInternalDevice(char *deviceMessage)
{
    logDebug("[Dispatcher] Trying dispatch command: %s", deviceMessage);

    if (!regexec(&switchRegex, deviceMessage, 0, NULL, 0))
    {
        logDebug("[Dispatcher] Found type: switch");
        return toggleSwitch(deviceMessage);
    }

    if (!regexec(&toggleButtonRegex, deviceMessage, 0, NULL, 0))
    {
        logDebug("[Dispatcher] Found type: toggleButton");
        return setNightWithness(deviceMessage);
    }

    if (!regexec(&wateringRegex, deviceMessage, 0, NULL, 0))
    {
        logDebug("[Dispatcher] Found type: watering");
        return triggerWatering(deviceMessage);
    }

    if (!regexec(&tankLevelRegex, deviceMessage, 0, NULL, 0))
    {
        logDebug("[Dispatcher] Found type:%stankLevel with id", deviceMessage);
        return triggerTankLevel(deviceMessage);
    }

    if (strcmp("heartbeat", deviceMessage) == 0)
    {
        logDebug("[Dispatcher] Heartbeat acknowledged", deviceMessage);
        return 1;
    }

    logDebug("[Dispatcher] Command %s not supported", deviceMessage);

    return 2;
}
