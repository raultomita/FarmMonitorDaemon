#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "watering.h"

redisAsyncContext *c;
struct event_base *base;

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {
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

void initializeRedis(void)
{
	signal(SIGPIPE, SIG_IGN);
    base = event_base_new();

    c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        printf("error: %s\n", c->errstr);
	}
	
	redisLibeventAttach(c, base);
}

void initializeExternalHandlers(void)
{   
    redisAsyncCommand(c, onMessage, NULL, "SUBSCRIBE commands");
    event_base_dispatch(base);
}

void onNotification(redisAsyncContext *c, void *reply, void *privdata) {
    if (reply == NULL) return;   
}


void sendNotification(char* message)
{
	redisAsyncCommand(c, onNotification, NULL, "SET tankLevel 40"); 
}
