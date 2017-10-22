#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"

const char *logFormat = "%s [%ld] %s\n";

void logMessage(char *type, const char *format, va_list args)
{
    char *message= (char *)malloc((strlen(logFormat) + 19 + strlen(format) + strlen(type)) * sizeof(char));
    sprintf(message, logFormat, type, pthread_self(), format);
    vprintf(message, args);
}

void logDebug(const char *format, ...)
{
    if (!debug)
        return;

    va_list args;
    va_start(args, format);
    logMessage("DBG", format, args);
    va_end(args);
}

void logInfo(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logMessage("INF", format, args);
    va_end(args);
}

void logError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logMessage("ERR", format, args);
    va_end(args);
}
