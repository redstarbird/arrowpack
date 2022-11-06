#ifndef REGEXFUNCTIONS_H
#define REGEXFUNCTIONS_H

#include <emscripten.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <regex.h>
#include "../C/StringRelatedFunctions.h"

typedef struct RegexMatch
{
    char *Text;
    unsigned int StartIndex, EndIndex;
} RegexMatch;

int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(char *Text, const char *Pattern); // returns the number of regex matches

RegexMatch EMSCRIPTEN_KEEPALIVE *GetAllRegexMatches(char *Text, const char *Pattern, unsigned int StartPos, unsigned int EndPos); // returns all regex matches as an array of strings

bool EMSCRIPTEN_KEEPALIVE HasRegexMatch(const char *text, const char *pattern);

void EMSCRIPTEN_KEEPALIVE regextest(char *text, const char *pattern);

#endif // !REGEXFUNCTIONS_H