#include <wiringPi.h>
#include <time.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "hiredis/adapters/libevent.h"

#include "tankLevel.h"
#include "watering.h"
#include "display.h"
#include "pins.h"

int ledPinTankFull = 23;

int commandPinTankInputEv = 17;
int commandPinTankOutputEv = 24;

int ledPinTankInputEvOperation = 18;
int ledPinTankOutputEvOperation = 25;

int btnPinDrain = 22;
int btnPinFill = 27;

int state = LOW;

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {
    redisReply *r = reply;
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY) {
		int j;
        for (j = 0; j < r->elements; j++) {
            printf("%u) %s\n", j, r->element[j]->str);
        }
    }
}

int main(void)
{
	wiringPiSetupGpio();
	initializeDisplay();
	
	 signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        printf("error: %s\n", c->errstr);
        return 1;
    }

    redisLibeventAttach(c, base);
    redisAsyncCommand(c, onMessage, NULL, "SUBSCRIBE testtopic");
    event_base_dispatch(base);
		
	initializeTankLevel();
	initializeWateringSchedule();
	

	time_t rawtime;
	struct tm* timeinfo;
	
	while(1)
	{
		delay(1000);
		
		state = !state;
		
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
				
		displayMessage(3, 0, asctime(timeinfo)); 
		
		timerCallbackWatering(timeinfo);
		timerCallbackTankLevel(timeinfo);	
	}
    return 0;
}
