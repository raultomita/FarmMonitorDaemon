#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "main.h"

pthread_mutex_t notificationMutex = PTHREAD_MUTEX_INITIALIZER;

redisContext *globalContext;

void onRedisConnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("On redis async connecting: %s", c->errstr);
        return;
    }
    logInfo("Redis async connected");
}

void onRedisDisconnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        logError("Redis async disconnecting: %s", c->errstr);
        return;
    }
    logInfo("Redis async disconnected");
}

redisContext *createSyncRedisConnection()
{
    redisContext *c = redisConnect(redisHost, redisPort);
    if (c == NULL || c->err)
    {
        if (c)
        {
            logError("Error in initializing redis sync connection: %s", c->errstr);
        }
        else
        {
            logError("Can't allocate redis context");
        }
    }
    else
    {
        return c;
    }
    return NULL;
}

char *getDeviceState(char *deviceId)
{
    redisContext *deviceStateContext = createSyncRedisConnection();

    if (deviceStateContext == NULL)
    {
        logError("DeviceStateContext is null");
        return NULL;
    }

    redisReply *r = redisCommand(deviceStateContext, "HGET %s state", deviceId);
    freeReplyObject(r);
    redisFree(deviceStateContext);
    return r->str;
}

void onRedisCommandReceived(redisAsyncContext *c, void *reply, void *privdata)
{
    redisReply *r = reply;
    if (reply == NULL)
        return;

    if (r->type == REDIS_REPLY_ARRAY)
    {
        logInfo("[Input] Command received");
        int j;
        for (j = 2; j < r->elements; j++)
        {
            if (r->element[j]->str != NULL)
            {
                triggerInternalDevice(r->element[j]->str);
            }
        }
    }
}

void sendMessage(int channel, char *key, char *data)
{
    if (globalContext == NULL)
    {
        logError("[Portal] GlobalContext is null");
        return;
    }

    logInfo("[Portal] Aquiring lock in order to send message");
    pthread_mutex_lock(&notificationMutex);

    redisReply *reply;
    switch (channel)
    {
    case NOTIFICATION:
        logInfo("[Portal] Save notification %s in devices and publish", key);
        reply = redisCommand(globalContext, "HSET devices %s %s", key, data);
        freeReplyObject(reply);
        reply = redisCommand(globalContext, "PUBLISH notifications %s", data);
        freeReplyObject(reply);
        logInfo("[Portal] Notification sent to %s", key);
        break;
    case COMMAND:
        logInfo("[Portal] Send command to %s", key);
        reply = redisCommand(globalContext, "PUBLISH commands %s", key);
        freeReplyObject(reply);
        logInfo("[Portal] Command sent to %s", key);
        break;
    case SAVESTATE:
        logInfo("[Portal] Save state to %s", key);
        reply = redisCommand(globalContext, "HSET %s state %s", key, data);
        freeReplyObject(reply);
        logInfo("[Portal] State saved to %s", key);
        break;
    }

    pthread_mutex_unlock(&notificationMutex);
}

void *externalCommandsThreadHandler(void *threadId)
{
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    redisAsyncContext *c = redisAsyncConnect(redisHost, redisPort);
    if (c->err)
    {
        logError("[Input] Redis thread handler error: %s", c->errstr);
    }

    redisLibeventAttach(c, base);
    redisAsyncSetConnectCallback(c, onRedisConnected);
    redisAsyncSetDisconnectCallback(c, onRedisDisconnected);
    redisAsyncCommand(c, onRedisCommandReceived, NULL, "SUBSCRIBE commands");
    logInfo("[Input] Subscribed to commands channel");
    event_base_dispatch(base);
    pthread_exit(NULL);
}

void initializeRedisPortal(void)
{
    globalContext = createSyncRedisConnection();
}

void acceptIncommingMessages(void)
{
    pthread_t thread;
    pthread_create(&thread, NULL, externalCommandsThreadHandler, NULL);
}
