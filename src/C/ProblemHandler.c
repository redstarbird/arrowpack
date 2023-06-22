// This file contains functions for generating warning messages and error messages
#include "ProblemHandler.h"

// Exits the program with a fatal error
void ThrowFatalError(char *message, ...)
{
    va_list args;
    ColorRed();

    va_start(args, message);
    printf("Fatal Error: ");
    vprintf(message, args);
    ColorReset();

    // use va_arg and va_end to access and end the variable arguments
    va_end(args);

    exit(1);
}

// Prints and logs a warning message
void CreateWarning(char *message, ...)
{
    va_list args;

    va_start(args, message);

    ColorYellow();
    printf("Warning: ");
    vprintf(message, args);
    ColorNormal();
    va_end(args);
}