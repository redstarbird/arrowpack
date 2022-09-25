#ifndef REGEXFUNCTIONS_H
#define REGEXFUNCTIONS_H
#include <stdio.h>
#include <emscripten.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../C/StringRelatedFunctions.h"

int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(const char *Text, const char *Pattern); // returns the number of regex matches

char EMSCRIPTEN_KEEPALIVE **GetAllRegexMatches(char *Text, const char *Pattern, unsigned int StartPos, unsigned int EndPos); // returns all regex matches as an array of strings

bool EMSCRIPTEN_KEEPALIVE HasRegexMatch(const char *text, const char *pattern);

#endif // !REGEXFUNCTIONS_H