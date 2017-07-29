#include <pthread.h>
#include <time.h>

extern int ledPinTankFull;

extern int commandPinTankInputEv;
extern int commandPinTankOutputEv;

extern int ledPinTankInputEvOperation;
extern int ledPinTankOutputEvOperation;

extern int btnPinDrain;
extern int btnPinFill;

extern int state;
extern struct tm* timeinfo;

extern pthread_cond_t notificationCond;
extern pthread_mutex_t notificationMutex;
