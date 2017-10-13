#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"

const char *infoLogFormat = "INF [%ld] %s\n";
const char *errorLogFormat = "ERR [%ld] %s\n";

void logInfo(const char *format, ...)
{
    char *message = (char *)malloc((strlen(infoLogFormat) + 19 + strlen(format)) * sizeof(char));
    sprintf(message, infoLogFormat, (long)pthread_self(), format);

    va_list args;
    va_start(args, format);
    int result = printf(message, args);
    va_end(args);
}

void logError(const char *format, ...)
{
    char *message = (char *)malloc((strlen(errorLogFormat) + 19 + strlen(format)) * sizeof(char));
    sprintf(message, errorLogFormat, pthread_self(), format);

    va_list args;
    va_start(args, format);
    int result = printf(message, args);
    va_end(args);
}