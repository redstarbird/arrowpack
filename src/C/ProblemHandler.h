#ifndef PROBLEMHANDLER_H
#define PROBLEMHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "TextColors.h"

void ThrowFatalError(char *message, ...);
void CreateWarning(char *message, ...);

#endif // !PROBLEMHANDLERH