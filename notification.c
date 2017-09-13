#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "main.h"

char *key4save;
char *value4save;

void *listenForNotifications(void *threadId)
{
	redisContext *c = redisConnect("127.0.0.1", 6379);
	if (c == NULL || c->err)
	{
		if (c)
		{
			printf("Error in listening for notifications: %s\n", c->errstr);			
		}
		else
		{
			printf("Can't allocate redis context\n");
		}
	}
	else
	{
		printf("[%ld] Listening fo notifications\n", (long)pthread_self());
		pthread_mutex_lock(&notificationMutex);
		
		while (1)
		{
			pthread_cond_wait(&notificationCond, &notificationMutex);

			redisReply *reply;
			printf("Preparing to save %s with %s", key4save, value4save);
			reply = redisCommand(c, "HSET devices %s %s", key4save, value4save);
			freeReplyObject(reply);
			reply = redisCommand(c, "PUBLISH notifications %s", value4save);
			freeReplyObject(reply);

			printf("[%ld] Notification Sent\n", (long)pthread_self());
		}
		pthread_mutex_unlock(&notificationMutex);
	}

	redisFree(c);
	pthread_exit(NULL);
}

void initializeNotification(void)
{
	pthread_t threadId;
	pthread_create(&threadId, NULL, listenForNotifications, NULL);
}

void saveAndNotify(char *key, char *data)
{
	printf("[%ld] Wait for notification\n", (long)pthread_self());
	pthread_mutex_lock(&notificationMutex);

	key4save = key;
	value4save = data;

	printf("Notification received for %s with %s", key4save, value4save);
	pthread_cond_signal(&notificationCond);

	printf("[%ld] Notification signal sent\n", (long)pthread_self());
	pthread_mutex_unlock(&notificationMutex);
}
