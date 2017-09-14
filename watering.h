#include <time.h>

void addWatering(char *wateringId, char *display, char *location, int commandGpio, int notifyGpio);
void timerCallbackWatering(struct tm *timeinfo);
void triggerWatering(char *deviceId);