#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "watering.h"
#include "tankLevel.h"
#include "switch.h"

void onRedisConnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void onRedisDisconnected(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void onRedisMessageReceived(redisAsyncContext *c, void *reply, void *privdata)
{
    redisReply *r = reply;
    if (reply == NULL)
        return;
    printf("[%ld] Command received\n", (long)pthread_self());
    if (r->type == REDIS_REPLY_ARRAY)
    {
        int j;
        for (j = 2; j < r->elements; j++)
        {
            if (r->element[j]->str != NULL &&
                strcmp(r->element[j]->str, "watering1") == 0)
            {
                triggerWatering();
            }
            else if (r->element[j]->str != NULL &&
                     strcmp(r->element[j]->str, "tankLevel1") == 0)
            {
                triggerTankLevel();
            }
	    else if (r->element[j]->str != NULL && 
		     strcmp(r->element[j]->str, "switch") == 0)
   	{
 toggleSwitch(2);
        }
}
    }
}

void *listenForRedisCommands(void *threadId)
{
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err)
    {
        printf("error: %s\n", c->errstr);
    }

    redisLibeventAttach(c, base);
    redisAsyncSetConnectCallback(c, onRedisConnected);
    redisAsyncSetDisconnectCallback(c, onRedisDisconnected);
    redisAsyncCommand(c, onRedisMessageReceived, NULL, "SUBSCRIBE commands");
    printf("[%ld]: Redis configuration complete\n", (long)pthread_self());
    event_base_dispatch(base);
    pthread_exit(NULL);
}

void initializeExternalHandlers(void)
{
    pthread_t commandsThread;
    pthread_create(&commandsThread, NULL, listenForRedisCommands, NULL);
}
