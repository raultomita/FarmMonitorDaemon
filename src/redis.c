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

pthread_mutex_t redisMutex = PTHREAD_MUTEX_INITIALIZER;
volatile int instanceInitialized = 0;

redisAsyncContext *globalContext;
eredis_t *eredisContext;

void sendCommandToRedis(redisCallbackFn *fn, char *privdata, const char *format, ...)
{
    if (globalContext == NULL)
    {
        logInfo("[Redis] Command cannot be sent due to client not being connected (%s)", format);
        return;
    }

    va_list args;
    va_start(args, format);
    logInfo("[Redis] Aquiring lock in order to send command");
    pthread_mutex_lock(&redisMutex);
    redisvAsyncCommand(globalContext, fn, privdata, format, args);
    pthread_mutex_unlock(&redisMutex);
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
        logInfo("[Redis] Received data for %s of type %s", (char *)privdata, r->element[1]->str);
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
                logInfo("[Input] Command received: %s", r->element[j]->str);
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
        char *deviceId = (char *)malloc(strlen(r->element[1]->element[dataIndex]->str) * sizeof(char));
        strcpy(deviceId, r->element[1]->element[dataIndex]->str);
        logInfo("[Redis] Device %s is registered", deviceId);
        sendCommandToRedis(onDeviceDataReceived, (char *)deviceId, "HGETALL %s", deviceId);
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
void onRedisAsyncConnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("[Redis] Connection to server failed: %s", c->errstr);
        return;
    }

    logInfo("[Redis] Connected to server");
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
        redisAsyncContext *c = redisAsyncConnect(redisHost, redisPort);
        if (c->err)
        {
            logError("[Redis] External commands thread handler error: %s", c->errstr);
        }
        else
        {
            redisLibeventAttach(c, base);
            redisAsyncSetConnectCallback(c, onRedisAsyncConnected);
            redisAsyncSetDisconnectCallback(c, onRedisAsyncDisconnected);
            redisAsyncCommand(c, onExternalCommandReceived, NULL, "SUBSCRIBE commands");
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

            if (!instanceInitialized)
            {
                sendCommandToRedis(onInstanceDevicesReceived, NULL, "SSCAN %s 0", instanceId);
            }

            event_base_dispatch(base);
        }
        globalContext = NULL;
        logInfo("[Redis] Trying to reconnect general purpose context in 5 sec");
        sleep(5);
    }

    logInfo("[Redis] Exit general purpose thread");
    pthread_exit(NULL);
}

void initializeRedis(void)
{
    instanceId = (char *)malloc(512 * sizeof(char));
    gethostname(instanceId, 512);
    logInfo("[Redis] host name is %s", instanceId);

    eredisContext = eredis_new();

    eredis_r_max(eredisContext, 50);
    eredis_host_add(eredisContext, redisHost, redisPort);
    eredis_run_thr(eredisContext);

    pthread_t threadExternalCommands;
    pthread_t threadGeneralPuroose;
    pthread_create(&threadExternalCommands, NULL, externalCommandsThreadHandler, NULL);
    pthread_create(&threadGeneralPuroose, NULL, generalPurposeThreadHandler, NULL);
}

void sendMessage(int channel, char *key, char *data)
{
    logInfo("[Redis] Sending message %d and key %s", channel, key);

    switch (channel)
    {
    case NOTIFICATION:    
        logInfo("[Redis] Save notification %s in devices and publish", key);
        eredis_w_cmd(eredisContext, "HSET devices %s %s", key, data);
        eredis_w_cmd(eredisContext, "PUBLISH notifications %s", data);
        logInfo("[Redis] Notification sent to %s", key);
        break;
    case COMMAND:
        logInfo("[Redis] Send external command to %s", key);
        eredis_w_cmd(eredisContext, "PUBLISH commands %s", key);
        logInfo("[Portal] Command sent to %s", key);
        break;
    }
}
