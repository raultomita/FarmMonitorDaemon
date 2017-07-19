#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "pins.h"
char * key4save;
char * value4save;
void * listenForNotifications(void * threadId)
{
	printf("[%ld] Listening fo notifications\n", (long)pthread_self());
	pthread_mutex_lock(&notificationMutex);
	while(1)
	{
		pthread_cond_wait(&notificationCond, &notificationMutex);
		
		printf("[%ld] Notification Sent\n", (long)pthread_self());
	}
	pthread_mutex_unlock(&notificationMutex);
	pthread_exit(NULL);
}

void initializeNotification(void)
{
	pthread_t threadId;
	pthread_create(&threadId, NULL, listenForNotifications, NULL);	
}

void saveAndNotify(char * key, char * data)
{
	pthread_mutex_lock(&notificationMutex);
	
	key4save = key;
	value4save = data;
	
	pthread_cond_signal(&notificationCond);
	pthread_mutex_unlock(&notificationMutex);
}
