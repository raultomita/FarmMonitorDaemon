#include <pthread.h>
#include <stdio.h>

#include "pins.h"

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
	pthread_cond_signal(&notificationCond);
	pthread_mutex_unlock(&notificationMutex);
}
