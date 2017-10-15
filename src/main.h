#include <pthread.h>
#include <time.h>
#include <hiredis/hiredis.h>

extern char* instanceId;
extern char* redisHost;
extern int redisPort;
extern int state;

void getCurrentTimeInfo(char *timeString, int bufferSize);
int triggerInternalDevice(char *deviceMessage);
void triggerDevice(char *deviceMessage);

//Massages to the other worlds
#define NOTIFICATION 1
#define COMMAND 2 
#define SAVESTATE 3

void initializeRedis(void);
void requestDeviceState(char *deviceId);
void sendMessage(int channel, char * key, char * data);
void initializeDevice(char *deviceId, redisReply *r);

//Diagnostic && Logging
void logInfo (const char * format, ... );
void logError (const char * format, ... );
