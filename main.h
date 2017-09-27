#include <pthread.h>
#include <time.h>

extern char* redisHost;
extern int redisPort;
extern int state;

void getCurrentTimeInfo(char *timeString, int bufferSize);

//Switch public methods
void toggleSwitch(char *switchId);
void addSwitch(char *switchId, char *display, char *location, int gpio);

//Watering public methods
void addWatering(char *wateringId, char *display, char *location, int commandGpio, int notifyGpio);
void timerCallbackWatering(struct tm *timeinfo);
void triggerWatering(char *deviceId);

//Toggle button public methods
void addToggleButton(char *toggleButtonId, int gpio, char *targetDeviceId);

//Tank level public methods
void timerCallbackTankLevel(struct tm *timeinfo);
void triggerTankLevel(char *deviceId);
void addTankLevel(char *tankLevelId, char *display, char *location, int commandGpio, int notifyGpio, int levelGpio);

//Massages to the other worlds
#define NOTIFICATION = 1;
#define COMMAND = 2; 

void initializeRedisPortal(void);
void acceptIncommingMessages(void);
void sendMessage(int type, char * key, char * data);