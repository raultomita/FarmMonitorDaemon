#include <pthread.h>
#include <time.h>

extern char* redisHost;
extern int redisPort;
extern int state;

void getCurrentTimeInfo(char *timeString, int bufferSize);
int triggerInternalDevice(char *deviceMessage);
void triggerDevice(char *deviceMessage);

//Massages to the other worlds
#define NOTIFICATION = 1;
#define COMMAND = 2; 

void initializeRedisPortal(void);
void acceptIncommingMessages(void);
void sendMessage(int type, char * key, char * data);