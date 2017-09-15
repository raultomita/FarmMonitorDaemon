#include <time.h>

void timerCallbackTankLevel(struct tm *timeinfo);
void triggerTankLevel(char *deviceId);
void addTankLevel(char *tankLevelId, char *display, char *location, int commandGpio, int notifyGpio, int levelGpio);
