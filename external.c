#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <wiringPi.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "watering.h"

void onRedisConnected(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void onRedisDisconnected(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void onRedisMessageReceived(redisAsyncContext *c, void *reply, void *privdata) {
    redisReply *r = reply;
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY) {
		int j;
        for (j = 2; j < r->elements; j++) {
            if(r->element[j]->str != NULL)
			{
				triggerWatering();
			}
        }
    }
}

PI_THREAD (redisCommandsThread)
{
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) 
    {
        printf("error: %s\n", c->errstr);
    }
	
	redisLibeventAttach(c, base);
	redisAsyncSetConnectCallback(c,onRedisConnected);
        redisAsyncSetDisconnectCallback(c,onRedisDisconnected);
	redisAsyncCommand(c, onRedisMessageReceived, NULL, "SUBSCRIBE commands");
printf("redis complete\n");
        event_base_dispatch(base);
return 0;
}

void initializeExternalHandlers(void)
{
   piThreadCreate(redisCommandsThread);
}

void sendNotification(char* message)
{
	
}
