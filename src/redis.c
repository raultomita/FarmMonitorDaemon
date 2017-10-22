#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <eredis.h>

#include "main.h"

volatile int instanceInitialized = 0;

redisAsyncContext *subscriberContext;
eredis_t *globalContext;

//-------
//Command templates
//
const char* publishCommandTemplate = "PUBLISH commands %s";

void sendCommandToRedis(redisCallbackFn *fn, char *privdata, const char *format, ...)
{
    if (subscriberContext == NULL)
    {
        logError("[Redis] Command cannot be sent due to client not being connected (%s)", format);
        return;
    }

    va_list args;
    va_start(args, format);
    
    redisvAsyncCommand(subscriberContext, fn, privdata, format, args);
    
    va_end(args);    
}

//-------
//Instance data initialization handlers
//-------
void dataHandler_externalCommand(redisAsyncContext *c, void *reply, void *privdata)
{
    redisReply *r = reply;
    if (reply == NULL)
    {
        logError("[Redis] reply in callback is null");
        return;
    }

    if (r->type == REDIS_REPLY_ARRAY)
    {
        int j;
        for (j = 2; j < r->elements; j++)
        {
            if (r->element[j]->str != NULL)
            {
                logDebug("[Input] Command received: %s", r->element[j]->str);
                triggerInternalDevice(r->element[j]->str);
            }
        }
    }
}

void dataHandler_deviceData(redisAsyncContext *c, void *reply, void *privdata)
{
    redisReply *r = reply;
    if (reply == NULL)
    {
        logError("[Redis] reply in callback is null");
        return;
    }

    if (r->type == REDIS_REPLY_ARRAY &&
        r->elements > 1 &&
        strcmp(r->element[0]->str, "type") == 0)
    {
        char *deviceId = privdata;
        logDebug("[Redis] Received data for device %s of type %s", (char *)privdata, r->element[1]->str);
        addDevice(deviceId, r);
    }
}

void subscribeForCommands()
{
    sendCommandToRedis(dataHandler_externalCommand, NULL, "SUBSCRIBE commands");
}

void dataHandler_InstanceDeviceIds(redisAsyncContext *c, void *reply, void *privdata)
{
    redisReply *r = reply;
    if (reply == NULL)
    {
        logError("[Redis] reply in callback is null");
        return;
    }

    if (r->type != REDIS_REPLY_ARRAY)
    {
        logError("[Redis] Response for SSCAN is not an array");
        return;
    }
    else if (r->elements != 2)
    {
        logError("[Redis] Array does not contains two elements");
        return;
    }
    else if (r->element[1] == NULL || r->element[1]->type != REDIS_REPLY_ARRAY)
    {
        logError("[Redis] Second element from SSCAN is not an array with device ids");
        return;
    }

    int dataIndex;
    for (dataIndex = 0; dataIndex < r->element[1]->elements; dataIndex++)
    {
        char *deviceId = (char *)malloc(strlen(r->element[1]->element[dataIndex]->str) * sizeof(char));
        strcpy(deviceId, r->element[1]->element[dataIndex]->str);
        logDebug("[Redis] Device %s is assigned to this instance. Request its data.", deviceId);
        sendCommandToRedis(dataHandler_deviceData, (char *)deviceId, "HGETALL %s", deviceId);
    }

    if (strcmp(r->element[0]->str, "0") != 0)
    {
        logDebug("[Redis] Request next page of assiged devices (cursor: %s).", r->element[0]->str);
        sendCommandToRedis(dataHandler_InstanceDeviceIds, NULL, "SSCAN %s %s", instanceId, r->element[0]->str);
    }
    else
    {
        instanceInitialized = 1;
        logDebug("[Redis] All devices are received, subscribe for commands");
        subscribeForCommands();
    }
}

//-------
//Redis connection management
//-------
void subscriberContextConnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("[Redis] Subscriber context failed to connetct: %s", c->errstr);
        return;
    }

    if (!instanceInitialized)
    {
        logInfo("[Redis] Subscriber context connected and starting initializing");
        sendCommandToRedis(dataHandler_InstanceDeviceIds, NULL, "SSCAN %s 0", instanceId);
    }
    else
    {
        logInfo("[Redis] Subscriber context connected again, subscribe to commands");
        subscribeForCommands();
    }
}

void subscriberContextDisconnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("[Redis] Subscriber context disconnected with error: %s", c->errstr);
        return;
    }
    logInfo("[Redis] Subscriber context disconnected");
}

void *subscriberThreadHandler(void *threadId)
{
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    logDebug("[Redis] External commands thread started");

    while (1)
    {
        subscriberContext = redisAsyncConnect(redisHost, redisPort);
        if (subscriberContext->err)
        {
            logError("[Redis] External commands thread handler error: %s", subscriberContext->errstr);
        }
        else
        {
            redisLibeventAttach(subscriberContext, base);
            redisAsyncSetConnectCallback(subscriberContext, subscriberContextConnected);
            redisAsyncSetDisconnectCallback(subscriberContext, subscriberContextDisconnected);

            event_base_dispatch(base);
        }
        subscriberContext = NULL;

        logDebug("[Redis] Trying to reconnect external commands in 5 sec");
        sleep(5);
    }

    logInfo("[Redis] Exit external commands thread");
    pthread_exit(NULL);
}

void initializeRedis(void)
{
    instanceId = (char *)malloc(512 * sizeof(char));
    gethostname(instanceId, 512);
    logInfo("[Redis] Instance name is %s", instanceId);

    globalContext = eredis_new();

    eredis_r_max(globalContext, 50);
    eredis_host_add(globalContext, redisHost, redisPort);
    eredis_run_thr(globalContext);

    pthread_t subscriberThread;
    pthread_create(&subscriberThread, NULL, subscriberThreadHandler, NULL);
}

void sendNotification(char *key, char *data)
{
    logDebug("[Redis] Sending notification to %d: %s", key, data);

    eredis_w_cmd(globalContext, "HSET devices %s %s", key, data);
    eredis_w_cmd(globalContext, "PUBLISH notifications %s", data);

    logDebug("[Redis] Notification sent", key);
}

void sendCommand(char *command, ...)
{
    va_list args;
    va_start(args, command);
    
    char *commandPattern = (char *)malloc((sizeof(command) + sizeof(publishCommandTemplate)) * sizeof(char));
    sprintf(commandPattern, publishCommandTemplate, command);

    logDebug("[Redis] Sending command %s", commandPattern);

    eredis_w_vcmd(globalContext, commandPattern, args);
    
    va_end(args);       
    
    logDebug("[Portal] Command sent");
}
