#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"

const char * infoLogFormat = "INFO  [%ld] %s\n";

int logInfo (const char * format, ... )
{
    char *message = (char *)malloc((strlen(infoLogFormat) + 12 + strlen(format)) * sizeof(char));
    va_list args;
    va_start(args, format);
    sprintf(message, infoLogFormat,  (long)pthread_self(), format);
    int result =  printf(message, args);
    va_end(args);
    return result;
}

int logError (const char * format, ... )
{
return 0;
}
