#ifndef REGEXFUNCTIONS_H
#define REGEXFUNCTIONS_H
#include <stdio.h>
#include <emscripten.h>
#include <regex.h>

int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(const char *Text, const char *Pattern); // returns the number of regex matches

char EMSCRIPTEN_KEEPALIVE **GetAllRegexMatches(char *Text, const char *Pattern); // returns all regex matches as an array of strings

#endif // !REGEXFUNCTIONS_H