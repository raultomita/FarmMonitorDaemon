#include <time.h>

//Switch public methods
void addSwitch(char *switchId, char *display, char *location, int gpio);
int toggleSwitch(char *switchId);

//Watering public methods
void addWatering(char *wateringId, char *display, char *location, int commandGpio, int notifyGpio);
int triggerWatering(char *deviceId);
void timerCallbackWatering(struct tm *timeinfo);

//Toggle button public methods
void addToggleButton(char *toggleButtonId, int gpio, int ledGpio, char *targetDeviceId);
int setNightWithness(char *targetDeviceId);

//Tank level public methods
void addTankLevel(char *tankLevelId, char *display, char *location, int commandGpio, int notifyGpio, int levelGpio);
int triggerTankLevel(char *deviceId);
void timerCallbackTankLevel(struct tm *timeinfo);
