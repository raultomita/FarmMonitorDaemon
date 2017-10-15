#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "main.h"

pthread_mutex_t redisMutex = PTHREAD_MUTEX_INITIALIZER;
volatile int instanceInitialized = 0;

redisAsyncContext *globalContext;

void sendCommandToRedis(redisCallbackFn *fn, void *privdata, const char *format, ...)
{
    if (globalContext == NULL)
    {
        logInfo("[Redis] Command cannot be sent due to client not being connected (%s)", format);
        return;
    }
    va_list args;
    va_start(args, format);
    redisvAsyncCommand(globalContext, fn, privdata, format, args);
    va_end(args);
    logInfo("[Redis] %s", format);
}

//-------
//Redis command handlers
//-------
void onRedisCommandSent(redisAsyncContext *c, void *reply, void *privdata)
{
    if (reply == NULL)
    {
        logError("[Redis] reply in callback is null");
        return;
    }

    logInfo("[Redis] command sent");
}

void onDeviceStateReceived(redisAsyncContext *c, void *reply, void *privdata)
{
    if (reply == NULL)
    {
        logError("[Redis] reply in callback is null");
        return;
    }

    char *deviceId = privdata;
    logInfo("[Redis] recceived state for %s", deviceId);
}

void onDeviceDataReceived(redisAsyncContext *c, void *reply, void *privdata)
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
        logInfo("[Redis] Received data for %s of type %s", deviceId, r->element[1]->str);
        initializeDevice(deviceId, r);
    }
}

void onExternalCommandReceived(redisAsyncContext *c, void *reply, void *privdata)
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
                logInfo("[Input] Command received");
                triggerInternalDevice(r->element[j]->str);
            }
        }
    }
}

void onInstanceDevicesReceived(redisAsyncContext *c, void *reply, void *privdata)
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
        char *deviceId = r->element[1]->element[dataIndex]->str;
        logInfo("[Redis] Device %s is registered", r->element[1]->element[dataIndex]->str);
        sendCommandToRedis(onDeviceDataReceived, deviceId, "HGETALL %s", deviceId);
    }

    //end
    if (strcmp(r->element[0]->str, "0") != 0)
    {
        sendCommandToRedis(onInstanceDevicesReceived, NULL, "SSCAN %s %s", instanceId, r->element[0]->str);
    }
    else
    {
        instanceInitialized = 1;
    }
}

//-------
//Redis connection management
//-------
void onRedisExternalCommandsConnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("[Redis] Connection to server failed: %s", c->errstr);
        return;
    }

    logInfo("[Redis] External commands context connected to server");

    redisAsyncCommand(c, onExternalCommandReceived, NULL, "SUBSCRIBE commands");
}

void onRedisGeneralPurposeConnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("[Redis] Connection to server failed: %s", c->errstr);
        return;
    }

    logInfo("[Redis] Connected to server");

    if (!instanceInitialized)
    {
        sendCommandToRedis(onInstanceDevicesReceived, NULL, "SSCAN %s 0", instanceId);
    }
}

void onRedisAsyncDisconnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("[Redis] Disconnected from server with: %s", c->errstr);
        return;
    }
    logInfo("[Redis] Disconnected from server");
}

void *externalCommandsThreadHandler(void *threadId)
{
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    logInfo("[Redis] External commands thread started");

    while (1)
    {
        redisAsyncConnect *c = redisAsyncConnect(redisHost, redisPort);
        if (c->err)
        {
            logError("[Redis] External commands thread handler error: %s", c->errstr);
        }
        else
        {
            redisLibeventAttach(c, base);
            redisAsyncSetConnectCallback(c, onRedisAsyncConnected);
            redisAsyncSetDisconnectCallback(c, onRedisAsyncDisconnected);
            event_base_dispatch(base);
        }

        logInfo("[Redis] Trying to reconnect external commands in 5 sec");
        sleep(5);
    }

    logInfo("[Redis] Exit external commands thread");
    pthread_exit(NULL);
}

void *generalPurposeThreadHandler(void *threadId)
{
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    logInfo("[Redis] General purpose thread started");

    while (1)
    {
        globalContext = redisAsyncConnect(redisHost, redisPort);
        if (globalContext->err)
        {
            logError("[Redis] General purpose thread handler error: %s", globalContext->errstr);
        }
        else
        {
            redisLibeventAttach(globalContext, base);
            redisAsyncSetConnectCallback(globalContext, onRedisAsyncConnected);
            redisAsyncSetDisconnectCallback(globalContext, onRedisAsyncDisconnected);
            event_base_dispatch(base);
        }
        globalContext = NULL;
        logInfo("[Redis] Trying to reconnect general purpose context in 5 sec");
        sleep(5);
    }

    logInfo("[Redis] Exit general purpose thread");
    pthread_exit(NULL);
}

void sendMessage(int channel, char *key, char *data)
{
    logInfo("[Redis] Sending message");

    switch (channel)
    {
    case NOTIFICATION:
        logInfo("[Redis] Save notification %s in devices and publish", key);
        sendCommandToRedis(onRedisCommandSent, NULL, "HSET devices %s %s", key, data);
        sendCommandToRedis(onRedisCommandSent, NULL, "PUBLISH notifications %s", data);
        logInfo("[Redis] Notification sent to %s", key);
        break;
    case COMMAND:
        logInfo("[Redis] Send external command to %s", key);
        sendCommandToRedis(onRedisCommandSent, NULL, "PUBLISH commands %s", key);
        logInfo("[Portal] Command sent to %s", key);
        break;
    case SAVESTATE:
        logInfo("[Portal] Save state to %s", key);
        sendCommandToRedis(onRedisCommandSent, NULL, "HSET %s state %s", key, data);
        logInfo("[Portal] State saved to %s", key);
        break;
    }
}

void requestDeviceState(char *deviceId)
{
    sendCommandToRedis(onDeviceStateReceived, deviceId, "HGET %s state", deviceId);
}

void initializeRedis(void)
{
    pthread_t threadExternalCommands;
    pthread_t threadGeneralPuroose;
    pthread_create(&threadExternalCommands, NULL, externalCommandsThreadHandler, NULL);
    pthread_create(&threadGeneralPuroose, NULL, generalPuposeThreadHandler, NULL);
}
