#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "watering.h"

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {
    redisReply *r = reply;
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY) {
		int j;
        for (j = 2; j < r->elements; j++) {
            if(r->element[j]->str != null)
			{
				triggerWatering();
			}
        }
    }
}

void initializeCommandHandlers(void)
{
	signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        printf("error: %s\n", c->errstr);
        return 1;
    }

    redisLibeventAttach(c, base);
    redisAsyncCommand(c, onMessage, NULL, "SUBSCRIBE commands");
    event_base_dispatch(base);
}