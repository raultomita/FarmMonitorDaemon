#include <pthread.h>

extern int ledPinTankFull;

extern int commandPinTankInputEv;
extern int commandPinTankOutputEv;

extern int ledPinTankInputEvOperation;
extern int ledPinTankOutputEvOperation;

extern int btnPinDrain;
extern int btnPinFill;

extern int state;

void getCurrentTimeInfo(char * timeString, int bufferSize);

extern pthread_cond_t notificationCond;
extern pthread_mutex_t notificationMutex;
