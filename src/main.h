#include <time.h>
#include <hiredis/hiredis.h>

extern char* instanceId;
extern char* redisHost;
extern int redisPort;
extern int state;
extern int debug;

void getCurrentTimeInfo(char *timeString, int bufferSize);

//Dispatcher
int triggerInternalDevice(char *deviceMessage);
void addDevice(char *deviceId, redisReply *r);
void initializeDispatcher();

//Redis api
void initializeRedis(void);
void sendNotification(char * key, char * data);
void sendCommand(char *command, ...);


//Diagnostic && Logging
void logDebug (const char * format, ... );
void logInfo (const char * format, ... );
void logError (const char * format, ... );
