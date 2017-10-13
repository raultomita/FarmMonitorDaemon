#include <pthread.h>

#include "main.h"

const char * infoLogFormat = "INFO  [%ld] %s\n";

int logInfo (const char * format, ... )
{
    char *message = (char *)malloc((strlen(infoLogFormat) + 12 + strlen(format)) * sizeof(char));
    sprintf(message, infoLogFormat,  (long)pthread_self(), format);
    printf(message, ...);
}

int logError (const char * format, ... )
{

}