#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "main.h"

pthread_cond_t notificationCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t notificationMutex = PTHREAD_MUTEX_INITIALIZER;

int NotificationsChannel = 1; //= "notifications";
int CommandsChannel = 2;

void initializeRedis(void)
{
    pthread_t thread;
    pthread_create(&thread, NULL, threadHandler_messages, NULL);

    pthread_t receiveCommandsThread;
    pthread_create(&receiveCommandsThread, NULL, listenForCommands, NULL);    
}

char *key4save;
char *value4save;
int channel;

void sendMessage(int type, char * key, char * data)
{
    printf("[%ld] Ready to send a message through portal\n", (long)pthread_self());
    pthread_mutex_lock(&notificationMutex);

    channel = type;
    key4save = key;
    value4save = data;

    pthread_cond_signal(&notificationCond);
    pthread_mutex_unlock(&notificationMutex);
}

void *messagesThreadHandler(void *threadId)
{
    redisContext *c = redisConnect(redisHost, redisPort);
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
            reply = redisCommand(c, "PUBLISH %s %s",NotificationChannel,  value4save);
            freeReplyObject(reply);

            printf("[%ld] Notification Sent\n", (long)pthread_self());
        }
        pthread_mutex_unlock(&notificationMutex);
    }

    redisFree(c);
    pthread_exit(NULL);
}

void *listenForCommands(void *threadId)
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
    redisAsyncCommand(c, onRedisCommandReceived, NULL, "SUBSCRIBE commands");
    printf("[%ld]: Subscribed to commands channel\n", (long)pthread_self());
    event_base_dispatch(base);
    pthread_exit(NULL);
}







void onRedisCommandReceived(redisAsyncContext *c, void *reply, void *privdata)
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
            if (r->element[j]->str != NULL)
            {
                if (strncmp("tankLevel", r->element[j]->str, strlen("tankLevel")) == 0)
                {
                    triggerTankLevel(r->element[j]->str);
                }
                else if (strncmp("switch", r->element[j]->str, strlen("switch")) == 0)
                {
                    toggleSwitch(r->element[j]->str);
                }
                else if (strncmp("watering", r->element[j]->str, strlen("watering")) == 0)
                {
                    triggerWatering(r->element[j]->str);
                }
            }
        }
    }
}

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